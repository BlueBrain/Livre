#include <vmmlib/t3_hooi.hpp>
#include <vmmlib/t3_converter.hpp>
#include <vmmlib/tensor_mmapper.hpp>

using namespace vmml;

//********** PARAMETERS **********
#define BRICK_SIZE 64 // Must be a power of two
#define BRICK_TA_RANK 32 // Must be <= BRICK_SIZE (but it looks like the renderer wants it to be exactly 32)
#define BRICK_BORDER 2
#define TA_BORDER 4

#define I1 128 // Size along 1st dimension
#define I2 128 // Size along 2nd dimension
#define I3 128 // Size along 3rd dimension
#define I 128 // Side of the minimal power-of-2-sided bounding cube; set it to 2^ceil(log2(max(I1,I2,I3)))
#define N 2 // Number of levels in the octree; set it to log2(I/BRICK_SIZE)+1

std::string input_dir = "./datasets";
std::string input_file = "blah.raw";
std::string input_comp_file = "blah_comp.raw"; // Name of an intermediate file for storing the input in the internal format

std::string output_dir = "./output/blah"; // Where to put the result

std::string noise_dir = "";
std::string noise_file = ""; // Noise file, for discarding bricks, leave blank if unknown

typedef float T_input; // Format of the input file
typedef float T_internal; // Format of internal computations. FIXME: double does not work yet!
bool compute_errors = true; // Whether the RMSE errors should be computed after the cores are generated (it causes a significant slow-down)
//*************************************

#define BORDER (BRICK_BORDER + TA_BORDER)
#define N_NODES ((1-(1<<(3*N)))/-7) // Total number of nodes in the octree (geometric series)
#define MATRIX1 I // Used for the initial decomposition
#define MATRIX2 (BRICK_SIZE*((1<<N)-1)+2*BORDER*N)
#define MATRIX3 (BRICK_SIZE+2*BORDER)*((1<<N)-1)
#define MATRIX4 ((BRICK_SIZE+2*BRICK_BORDER)*((1<<N)-1)-2*(N-1)) // Rows in each final factor matrix

std::string input_comp_path;
typedef unsigned char T_output;
typedef tensor3< BRICK_SIZE + 2*BORDER, BRICK_SIZE + 2*BORDER, BRICK_SIZE + 2*BORDER, T_internal > brick_with_borders_t;
typedef matrix< BRICK_SIZE + 2*BORDER, BRICK_TA_RANK, T_internal > brick_matrix_t;
typedef tensor3< BRICK_TA_RANK, BRICK_TA_RANK, BRICK_TA_RANK, T_internal > core_t;

matrix< I + 2*BORDER, I + 2*BORDER, T_internal > *arrays[N][BRICK_SIZE + 2*BORDER]; // For each level, a circular array of slices, including borders. Slices are referred to as (level, index)
int indices[N]; // For each level, stores the index of the first slice not yet successfully obtained (i.e. either by reading or averaging)
matrix< N_NODES, 15, double > ranks; // There we store the info about each node, for the renderer. There are 15 fields per node
double noise_stdev = 0; // Threshold to discard blocks with only noise

// There are 4 kinds of matrices:
matrix< MATRIX1, BRICK_TA_RANK, T_internal > *matrices1[3]; // Global factor matrices
matrix< MATRIX2, BRICK_TA_RANK, T_internal > *matrices2[3]; // Mipmapped matrix pyramid
matrix< MATRIX3, BRICK_TA_RANK, T_internal > *matrices3[3]; // Result of applying per-brick SVD. Used to compress the bricks into cores
matrix< MATRIX4, BRICK_TA_RANK, T_internal > *matrices4[3]; // Result of removing TA borders. Final matrices that the renderer understands

void matrix1_to_matrix2(matrix< MATRIX1, BRICK_TA_RANK, T_internal > *matrix1, matrix< MATRIX2, BRICK_TA_RANK, T_internal > *matrix2) {
    matrix2->set_sub_matrix(*matrix1,MATRIX2-MATRIX1-BORDER,0); // First, paste the whole global matrix1
    matrix< 1, BRICK_TA_RANK, T_internal > row0, row1; // For manipulating rows

    for (int i = N-2; i >= 0; --i) { // Mipmap the remaining levels, from bigger to smaller
        int starting_row = BRICK_SIZE*(pow(2,i)-1) + i*2*BORDER + BORDER;
        int lower_starting_row = BRICK_SIZE*(pow(2,i+1)-1) + (i+1)*2*BORDER + BORDER;
        
        for (int j = 0; j < BRICK_SIZE*pow(2,i); ++j) {
            matrix2->get_sub_matrix(row0, lower_starting_row + j*2, 0);
            matrix2->get_sub_matrix(row1, lower_starting_row + j*2 + 1, 0);
            row0 += row1;
            row0 /= 2;
            matrix2->set_row(starting_row + j, row0);
        }
    }
}

void matrix2_to_matrix3(matrix< MATRIX2, BRICK_TA_RANK, T_internal > *matrix2, matrix< MATRIX3, BRICK_TA_RANK, T_internal > *matrix3) {
    matrix< BRICK_SIZE + 2*BORDER, BRICK_TA_RANK, T_internal > window; // As this window slides downward, SVD's are produced out of it
    lapack_svd< BRICK_SIZE + 2*BORDER, BRICK_TA_RANK, T_internal > svd;
    vector< BRICK_TA_RANK, T_internal > lambdas;
    int brick_number = 0;
    for (int i = 0; i < N; ++i) { // For every level
        int starting_row = BRICK_SIZE*(pow(2,i)-1) + i*2*BORDER;
        for (int j = 0; j < pow(2,i); ++j) { // For every brick in the level
            matrix2->get_sub_matrix(window, starting_row + j*BRICK_SIZE, 0);
            svd.compute_and_overwrite_input(window, lambdas);
            matrix3->set_sub_matrix(window, (BRICK_SIZE + 2*BORDER)*brick_number, 0);
            ++brick_number;
        }
    }
}

void matrix3_to_matrix4(matrix< MATRIX3, BRICK_TA_RANK, T_internal > *matrix3, matrix< MATRIX4, BRICK_TA_RANK, T_internal > *matrix4) {
    matrix< BRICK_SIZE + 2*BRICK_BORDER, BRICK_TA_RANK, T_internal > window; // As this window slides downward, it is copied to the matrix4
    int brick_number = 0;
    for (int i = 0; i < N; ++i) { // For every level
        for (int j = 0; j < pow(2,i); ++j) { // For every brick in the level
            matrix3->get_sub_matrix(window, (BRICK_SIZE + 2*BORDER)*brick_number + TA_BORDER, 0);
            matrix4->set_sub_matrix(window, (BRICK_SIZE + 2*BRICK_BORDER)*brick_number - 2*i, 0);
            ++brick_number;
        }
    }
}

// Generate the 3 factor matrices
void generate_matrices() {
    std::cout << "Generating global factor matrices..." << std::endl;

    // ALS to obtain the 3 factor matrices; they may have less than I rows, so we paste them afterwards into matrices1[]
    matrix< I1, BRICK_TA_RANK, T_internal > u1_small;
    matrix< I2, BRICK_TA_RANK, T_internal > u2_small;
    matrix< I3, BRICK_TA_RANK, T_internal > u3_small;
    
    // Memory-map the volume
    typedef t3_converter< I1, I2, I3, T_internal > t3_conv_t;
    t3_conv_t conv;
    tensor_mmapper< tensor3< I1, I2, I3, T_internal >, t3_conv_t > t3_mmap( input_dir, input_comp_file, true, conv );
    tensor3< I1, I2, I3, T_internal > volume;
    t3_mmap.get_tensor( volume );
    typedef t3_hooi< BRICK_TA_RANK, BRICK_TA_RANK, BRICK_TA_RANK, I1, I2, I3, T_internal > hooi_t;
    hooi_t::als( volume, u1_small, u2_small, u3_small, hooi_t::init_random(), 0.0, 3 );

    matrices1[0]->set_sub_matrix(u1_small, 0, 0);
    matrices1[1]->set_sub_matrix(u2_small, 0, 0);
    matrices1[2]->set_sub_matrix(u3_small, 0, 0);

    std::cout << "Processing matrices..." << std::endl;
    
    // Mipmap them and then add borders, to get matrices2[]
    matrix1_to_matrix2(matrices1[0],matrices2[0]);
    matrix1_to_matrix2(matrices1[1],matrices2[1]);
    matrix1_to_matrix2(matrices1[2],matrices2[2]);

    // SVD each brick, to get matrices3[]
    matrix2_to_matrix3(matrices2[0],matrices3[0]);
    matrix2_to_matrix3(matrices2[1],matrices3[1]);
    matrix2_to_matrix3(matrices2[2],matrices3[2]);
        
    // Remove the TA borders, to get matrices4[]
    matrix3_to_matrix4(matrices3[0],matrices4[0]);
    matrix3_to_matrix4(matrices3[1],matrices4[1]);
    matrix3_to_matrix4(matrices3[2],matrices4[2]);
    
    // Write the final matrices
    matrices4[0]->write_to_raw(output_dir,"u1.raw");
    matrices4[1]->write_to_raw(output_dir,"u2.raw");
    matrices4[2]->write_to_raw(output_dir,"u3.raw");
}

// Read a slice and store it into (N-1, index)
void read_slice(int slice, int index) {

    matrix< I1, I2, T_internal > *reading_slice = new matrix< I1, I2, T_internal >;
    reading_slice->zero();
    
    if (slice < I3) { // If we didn't read already everything
        std::cout << "We read the slice " << slice << " (" << I1 << " x " << I2 << ")" << std::endl;
        
        size_t len_data = sizeof (T_internal) * I1 * I2;
        size_t max_file_len = 2147483648u - sizeof (T_internal);
        size_t len_read = 0;
        size_t start_idx =  I1 * I2 * sizeof(T_internal) * slice; 
        
        std::ifstream infile;
        infile.open(input_comp_path.c_str(), std::ios::in);
        char* data = new char[ len_data ];
        T_internal *it = reading_slice->begin(),
        *it_end = reading_slice->end();
        
        while (len_data > 0) {
            len_read = (len_data % max_file_len) > 0 ? len_data % max_file_len : len_data;
            len_data -= len_read;

            infile.seekg( start_idx, std::ios::beg );
            if( (long unsigned int)(infile.tellg()) != start_idx )
            {
                std::cerr << "Can't proceed to the offset: " << start_idx << " to read file: " << input_comp_file << std::endl;
                infile.close();
            }

            infile.read(data, len_read);

            if ( infile.fail())
                std::cerr << "problem with reading..." << std::endl;

            T_internal* T_ptr = (T_internal*)&(data[0]);
            for (; (it != it_end) && (len_read > 0); ++it, len_read -= sizeof (T_internal)) {
                *it = *T_ptr;
                ++T_ptr;
            }
        }

        delete[] data;
        infile.close();
    }
    arrays[N-1][index%(BRICK_SIZE + 2*BORDER)]->set_sub_matrix(*reading_slice, BORDER, BORDER);
}

// Average (orig_level, orig_index1) and (orig_level, orig_index2) and put the result in (dest_level, dest_index)
void average_slices(int orig_level, int orig_index1, int orig_index2, int dest_level, int dest_index) {
    std::cout << "We average " << "(" << orig_level << ", " << orig_index1 << ") and (" << orig_level << ", " << orig_index2 << ") into (" << dest_level << ", " << dest_index << ")" << std::endl;

    orig_index1 = orig_index1%(BRICK_SIZE + 2*BORDER);
    orig_index2 = orig_index2%(BRICK_SIZE + 2*BORDER);
    dest_index = dest_index%(BRICK_SIZE + 2*BORDER);
    
    for (int i = 0; i < I/2; ++i) {
        for (int j = 0; j < I/2; ++j) {
            arrays[dest_level][dest_index]->at(i+BORDER,j+BORDER) = (
                    arrays[orig_level][orig_index1]->at(i*2+BORDER,j*2+BORDER) + 
                    arrays[orig_level][orig_index1]->at(i*2+BORDER,j*2+BORDER+1) + 
                    arrays[orig_level][orig_index1]->at(i*2+BORDER+1,j*2+BORDER) + 
                    arrays[orig_level][orig_index1]->at(i*2+BORDER+1,j*2+BORDER+1) + 
                    arrays[orig_level][orig_index2]->at(i*2+BORDER,j*2+BORDER) + 
                    arrays[orig_level][orig_index2]->at(i*2+BORDER,j*2+BORDER+1) + 
                    arrays[orig_level][orig_index2]->at(i*2+BORDER+1,j*2+BORDER) + 
                    arrays[orig_level][orig_index2]->at(i*2+BORDER+1,j*2+BORDER+1))/8;
        }
    }
}

// Compute a core id number that the renderer will understand
int compute_id(int level, int index, int brick) {
    int id = 0;
    for (int i = 0; i < level; ++i) {
        id += round(pow(8,i));
    }
    id += index*round(pow(4,level));
    id += brick;
    return id+1;
}

// Truncates a decomposition and returns the norm between its resulting reconstruction and the original. TODO: it's a provisional implementation; we should calculate the error with the original resolution, not with the downsampled one
template< size_t RANKS >
double compute_error(core_t& core, brick_matrix_t& u1, brick_matrix_t& u2, brick_matrix_t& u3, brick_with_borders_t& brick_with_borders) {
    tensor3< RANKS, RANKS, RANKS, T_internal > truncated_core;
    matrix< BRICK_SIZE + 2*BORDER, RANKS, T_internal > truncated_u1, truncated_u2, truncated_u3;
    
    core.get_sub_tensor3(truncated_core, 0, 0, 0);
    u1.get_sub_matrix(truncated_u1);
    u2.get_sub_matrix(truncated_u2);
    u3.get_sub_matrix(truncated_u3);
    
    brick_with_borders_t reconstruction;
    t3_ttm::full_tensor3_matrix_multiplication(truncated_core, truncated_u1, truncated_u2, truncated_u3, reconstruction);
    return reconstruction.frobenius_norm(brick_with_borders);
}

// Compress every brick of the block defined by taking (BRICK_SIZE + 2*BORDER) slices, starting at (level, index)
void compress_block(int level, int block, int index) {
    std::cout << "We compress, at level " << level << ", the block " << block << std::endl;
    int bricks_per_side = round(pow(2,level));

    int brick_number = 0;
    for (int j = 0; j < bricks_per_side; ++j) {
        for (int i = 0; i < bricks_per_side; ++i) {
            brick_with_borders_t brick_with_borders; // Used to copy the bricks and their borders
            for (int k = 0; k < BRICK_SIZE + 2*BORDER; ++k) {
                brick_with_borders.set_frontal_slice_fwd(k,arrays[level][(index + k)%(BRICK_SIZE + 2*BORDER)]->get_sub_matrix<BRICK_SIZE + 2*BORDER, BRICK_SIZE + 2*BORDER>(i*BRICK_SIZE, j*BRICK_SIZE));
            }
            tensor3< BRICK_SIZE, BRICK_SIZE, BRICK_SIZE, T_internal > clean_brick; // Used to compute the min, max and stdev (we don't want borders for that)
            brick_with_borders.get_sub_tensor3(clean_brick, BORDER, BORDER, BORDER);
            int id = compute_id(level,block,brick_number);
            ++brick_number;
            
            // Fill in the appropriate line of the ranks matrix
            ranks.at(id-1,0) = id; // Node id number
            ranks.at(id-1,5) = i*BRICK_SIZE; // x
            ranks.at(id-1,6) = j*BRICK_SIZE; // y
            ranks.at(id-1,7) = block*BRICK_SIZE; // z
            
            // Check bounds in irregular datasets
            int mult = I/round(pow(2,level));
            if (i*mult >= I1 or j*mult >= I2 or block*mult >= I3 or clean_brick.stdev() <= noise_stdev) { // Empty brick, or white noise: we skip its core generation
                ranks.at(id-1,1) = 0;
                continue;
            }
            ranks.at(id-1,1) = 32;
            ranks.at(id-1,2) = clean_brick.get_min();
            ranks.at(id-1,3) = clean_brick.get_max();

            // Core generation
            
            // Retrieve the brick factor matrices from matrices3[]
            brick_matrix_t u1, u2, u3;
            int row = (BRICK_SIZE + 2*BORDER)*(pow(2,level)-1);
            matrices3[0]->get_sub_matrix(u1, row + i*(BRICK_SIZE + 2*BORDER), 0);
            matrices3[1]->get_sub_matrix(u2, row + j*(BRICK_SIZE + 2*BORDER), 0);
            matrices3[2]->get_sub_matrix(u3, row + block*(BRICK_SIZE + 2*BORDER), 0);
            
            core_t core;
            t3_ttm::full_tensor3_matrix_multiplication(brick_with_borders, transpose(u1), transpose(u2), transpose(u3), core);
            
            if (compute_errors) { // RMSE error computations
                double brick_with_borders_norm = brick_with_borders.frobenius_norm();
                ranks.at(id-1,8) = compute_error<8>(core, u1, u2, u3, brick_with_borders)/brick_with_borders_norm;
                ranks.at(id-1,9) = compute_error<10>(core, u1, u2, u3, brick_with_borders)/brick_with_borders_norm;
                ranks.at(id-1,10) = compute_error<12>(core, u1, u2, u3, brick_with_borders)/brick_with_borders_norm;
                ranks.at(id-1,11) = compute_error<14>(core, u1, u2, u3, brick_with_borders)/brick_with_borders_norm;
                ranks.at(id-1,12) = compute_error<16>(core, u1, u2, u3, brick_with_borders)/brick_with_borders_norm;
                ranks.at(id-1,13) = compute_error<24>(core, u1, u2, u3, brick_with_borders)/brick_with_borders_norm;
                ranks.at(id-1,14) = compute_error<32>(core, u1, u2, u3, brick_with_borders)/brick_with_borders_norm;
            }

            // Quantize the core, to write it in the T_output format
            tensor3< BRICK_TA_RANK, BRICK_TA_RANK, BRICK_TA_RANK, T_output > tmp_quantized_core, quantized_core;
            tensor3< BRICK_TA_RANK, BRICK_TA_RANK, BRICK_TA_RANK, char > signs; 

            T_internal core_min, core_max;
            core.quantize_log( tmp_quantized_core, signs, core_min, core_max, T_output(127) );
            ranks.at(id-1,4) = core_max;

            //colume-first iteration
            //backward cylcling after lathauwer et al. 
            for (size_t r2 = 0; r2 < BRICK_TA_RANK; ++r2 ) {
                for (size_t r3 = 0; r3 < BRICK_TA_RANK; ++r3 ) {
                    for (size_t r1 = 0; r1 < BRICK_TA_RANK; ++r1 ) {
                        quantized_core.at( r1, r2, r3 ) = (tmp_quantized_core.at( r1, r2, r3 ) | (signs.at( r1, r2, r3) << 7 ));
                    }
                }
            }

            // We generate the core's name from the computed id, and save it there
            std::stringstream ss;
            ss << "core_" << id << ".raw";
            std::cout << "Saved " << ss.str() << std::endl;
            t3_converter< BRICK_TA_RANK, BRICK_TA_RANK, BRICK_TA_RANK, T_output >::write_to_raw(quantized_core, output_dir, ss.str());
//            t3_converter< BRICK_TA_RANK, BRICK_TA_RANK, BRICK_TA_RANK, float >::write_to_raw(core, output_dir, ss.str());
        }
    }
}

// Core of the algorithm: recursively traverse the tree. This function advances one position in the given level, i.e. computes a slice (either by reading or by averaging). If a block is just completed thanks to the newly computed slice, we compress it. We also use the new slice to generate as many further lower blocks as possible
void advance(int level) {

	if (level < 0) // Tree bottom already reached
		return;

	if (level == N-1) {
        read_slice(indices[N-1] - BORDER,indices[N-1]); // If we're in the uppermost level, get it from the data
	}
	else {
		average_slices(level+1, indices[level+1]-2, indices[level+1]-1, level, indices[level]); // Otherwise it is the average of two upper slices
	}
    indices[level]++; // We just generated a new slice
	if ((indices[level] - 2*BORDER)%BRICK_SIZE == 0 and (indices[level] - 2*BORDER)/BRICK_SIZE > 0) { // If we just completed a block, then we can compress it
		compress_block(level, (indices[level] - 2*BORDER)/BRICK_SIZE - 1, indices[level] - (BRICK_SIZE + 2*BORDER));
	}
    else if ((indices[level] - BORDER)%BRICK_SIZE == 0 and (indices[level] - BORDER)/BRICK_SIZE == pow(2,level)) { // Special case: it's the last block (we have to zero-pad first)
        for (int i = indices[level]; i < indices[level] + BORDER; ++i) {
            arrays[level][i%(BRICK_SIZE + 2*BORDER)]->zero();
        }
        compress_block(level, (indices[level] - BORDER)/BRICK_SIZE - 1, indices[level] - (BRICK_SIZE + BORDER));
    }
	if ((indices[level] - BORDER)%2 == 0)
		advance(level-1); // Every two blocks, we are ready to generate a block in the lower level
}

int main (int argc, char * const argv[]) {

    //********** Initialization ***********
    if (not noise_dir.empty() and not noise_file.empty()) { // If possible, get a noise stdev threshold
        std::cout << "Reading noise file... ";
        tensor3< BRICK_SIZE, BRICK_SIZE, BRICK_SIZE, T_input > noise_brick;
        t3_converter<  BRICK_SIZE, BRICK_SIZE, BRICK_SIZE, T_input >::read_from_raw(noise_brick, noise_dir, noise_file);
        noise_stdev = noise_brick.stdev(); // TODO stdev is not the best noise measure: with it, any brick that takes a constant value is discarded
        std::cout << "noise_stdev = " << noise_stdev << std::endl;
    }
    std::cout << "Initializing data structures..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        matrices1[i] = new matrix< MATRIX1, BRICK_TA_RANK, T_internal >;
        matrices1[i]->zero();
        
        matrices2[i] = new matrix< MATRIX2, BRICK_TA_RANK, T_internal >;
        matrices2[i]->zero();
        
        matrices3[i] = new matrix< MATRIX3, BRICK_TA_RANK, T_internal >;
        matrices3[i]->zero();
        
        matrices4[i] = new matrix< MATRIX4, BRICK_TA_RANK, T_internal >;
        matrices4[i]->zero();
    }

    for (int i = 0; i < N; ++i) {
        std::cerr << "Initializing level " << i << " ";
        for (int j = 0; j < BRICK_SIZE + 2*BORDER; ++j) {
            std::cerr << "*";
            arrays[i][j] = new matrix< I + 2*BORDER, I + 2*BORDER, T_internal >;
            arrays[i][j]->zero();
        }
        std::cerr << std::endl;
    }

    for (int i = 0; i < N; ++i)
        indices[i] = BORDER; // Because only the first border has been "calculated" (it's just zero)
    
    ranks.zero();
    //*************************************

    //*********** Algorithm run ***********
    input_comp_path = input_dir;
    if (input_comp_path[input_comp_path.length()-1] != '/') input_comp_path.append( "/" );
    input_comp_path.append(input_comp_file);
    std::ifstream ifile(input_comp_path.c_str());
    if (!ifile) { // We need a dataset version in T_internal format
        std::cerr << "Converting input file..." << std::endl;
        tensor3< I1, I2, I3, T_input > input;
        t3_converter< I1, I2, I3, T_input >().read_from_raw( input, input_dir, input_file );
        tensor3< I1, I2, I3, T_internal > output;
        output.cast_from( input );
        t3_converter< I1, I2, I3, T_internal >().write_to_raw( output, input_dir, input_comp_file );
    }
    
    generate_matrices();

    for (int i = 0; i < I; ++i) {
        advance(N-1); // Do all the slices in the uppermost level; the lower ones will get done recursively
    }
    
    ranks.write_to_raw(output_dir,"ranks.raw"); // Print out the ranks matrix
    ranks.write_csv_file(output_dir,"ranks.csv"); // For debugging
    //*************************************
}
