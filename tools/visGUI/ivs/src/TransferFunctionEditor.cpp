/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 * edited: Maxim Makhinya (maxmah@gmial.com)
 *         David Steiner (steiner@ifi.uzh.ch)
 **/

#include "TransferFunctionEditor.h"

#include <QFileDialog>

#include "IO/TransferFunctionFile.h"
#include "../../mainWindow.h"

using namespace std;

namespace ivs
{


TransferFunctionEditor::TransferFunctionEditor( massVolGUI::MainWindow *_mainWnd, QWidget *_parent ):
  QDialog(_parent),
///  directory_(QString::fromStdString(_ivsWidget->getDataPath())),
  transferFunction_(0),
  rgba_(0),
  sda_(0),
  histogram_(0),
  rgbaSize_(0),
  sdaSize_(0),
  histogramSize_(0),
  mainWnd_(_mainWnd),
  globalRank_( 0 ),
  ranks_( 0 ),
  updateMaxSet_( true )
{
  ui_.setupUi(this);
  QList<QPushButton *> buttonList = findChildren<QPushButton *>();
    foreach(QPushButton *pb, buttonList) {
        pb->setDefault( false );
        pb->setAutoDefault( false );
    }

  setWindowFlags((windowFlags() ^ Qt::Dialog) | Qt::Window);
  position_ = pos();
  transferFunctionGraph_ = new FreehandGraph();
  initializeGraph(transferFunctionGraph_);
  connectTensorRanks();
  timer_.start();

  ranksSpinBoxList_.append( ui_.spinBoxRank0_ );
  ranksSpinBoxList_.append( ui_.spinBoxRank1_ );
  ranksSpinBoxList_.append( ui_.spinBoxRank2_ );
  ranksSpinBoxList_.append( ui_.spinBoxRank3_ );
  ranksSpinBoxList_.append( ui_.spinBoxRank4_ );
  ranksSpinBoxList_.append( ui_.spinBoxRank5_ );
  ranksSpinBoxList_.append( ui_.spinBoxRank6_ );
  ranksSpinBoxList_.append( ui_.spinBoxRank7_ );
  ranksSpinBoxList_.append( ui_.spinBoxRank8_ );
  ranksSpinBoxList_.append( ui_.spinBoxRank9_ );

  autoUpdateRanks_ = ui_.checkBoxTensorAutoUpdate_->isChecked();
}

TransferFunctionEditor::~TransferFunctionEditor()
{
  delete transferFunctionGraph_;
  delete [] rgba_;
  delete [] sda_;
}


void TransferFunctionEditor::connectTensorRanks()
{
  connect( ui_.spinBoxRankGlobal_, SIGNAL( valueChanged(int) ), this, SLOT( changeGlobalRank(int) ));
  connect( ui_.spinBoxRank0_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));
  connect( ui_.spinBoxRank1_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));
  connect( ui_.spinBoxRank2_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));
  connect( ui_.spinBoxRank3_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));
  connect( ui_.spinBoxRank4_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));
  connect( ui_.spinBoxRank5_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));
  connect( ui_.spinBoxRank6_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));
  connect( ui_.spinBoxRank7_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));
  connect( ui_.spinBoxRank8_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));
  connect( ui_.spinBoxRank9_     , SIGNAL( valueChanged(int) ), this, SLOT( changeRank(int) ));

  connect( ui_.checkBoxTensorAutoUpdate_, SIGNAL( stateChanged(int) ), this, SLOT( changeRankAutoUpdate(int) ));
  connect( ui_.pushButtonTensorUpdate_,   SIGNAL( clicked()),          this, SLOT( updateTensorRanks() ));
}

void TransferFunctionEditor::updateTensorRanks( bool force )
{
  bool changed = force;
  assert( ranks_->size() == ranksLastCommit_.size() );
  for( size_t i = 0; i < ranksLastCommit_.size(); ++i )
    if( (*ranks_)[i] != ranksLastCommit_[i] )
    {
        changed = true;
        break;
    }
  if( changed )
  {
    ranksLastCommit_ = *ranks_;
    mainWnd_->updateTensorRanks();
  }
}

void TransferFunctionEditor::changeRankAutoUpdate( int state )
{
  autoUpdateRanks_ = (state == Qt::Checked);
  if( autoUpdateRanks_ )
    updateTensorRanks( false );
}

void TransferFunctionEditor::changeRank( int value )
{
  int i = 0;
  foreach( QSpinBox *sb, ranksSpinBoxList_ )
  {
    if( (QSpinBox *)sender() == sb )
    {
        changeRank( value, i );
        return;
    }
    ++i;
  }
}

void TransferFunctionEditor::changeGlobalRank( int value )
{
  bool changed    = false;
  bool autoUpdate  = autoUpdateRanks_;
  autoUpdateRanks_ = false;
  updateMaxSet_    = false;

  *globalRank_ = value;
  int i = 0;
  foreach( QSpinBox *sb, ranksSpinBoxList_ )
  {
    const byte oldValue = (*ranks_)[i];
    assert( sb->value() == oldValue );
    sb->setMaximum( value );

    if( oldValue > value )
    {
        (*ranks_)[i] = value;
        changed = true;
    }else
        if( (*ranks_)[i] <= ranksMaxSet_[i] && 
            (*ranks_)[i] < value )
        {
            (*ranks_)[i] = qMin( (int)ranksMaxSet_[i], value );
            sb->setValue( (*ranks_)[i] );
            changed = true;
        }
    ++i;
  }

  autoUpdateRanks_ = autoUpdate;
  updateMaxSet_    = true;

  if( changed && autoUpdateRanks_ )
    updateTensorRanks( false );
}


void TransferFunctionEditor::changeRank( int value, int rank )
{
  assert( rank >= 0 && rank <= (int)ranks_->size() );
  (*ranks_)[rank] = value;

  if( updateMaxSet_ )
    ranksMaxSet_[rank] = value;

  if( autoUpdateRanks_ )
    updateTensorRanks( false );
}

void TransferFunctionEditor::setTensorRanks( byte* globalRank, std::vector<byte>* ranks )
{
  std::cout << "Set Tensor Parameters: "; for( size_t i = 0; i < ranks->size(); ++i )  std::cout << (int)((*ranks)[i]) << ", "; std::cout << std::endl;

  globalRank_ = globalRank;
  ranks_      = ranks;
  backupTensorRanks();

  ranksMaxSet_ = *ranks;

  bool autoUpdate = autoUpdateRanks_;
  autoUpdateRanks_ = false;
  assert( (int)ranks->size() >= ranksSpinBoxList_.size() );
  int i = 0;
  foreach( QSpinBox *sb, ranksSpinBoxList_ )
  {
    if( (*ranks)[i] > *globalRank )
        (*ranks)[i] = *globalRank;

    sb->setValue( (*ranks)[i] );
    ++i;
  }
  ui_.spinBoxRankGlobal_->setValue( *globalRank );
  autoUpdateRanks_ = autoUpdate;
}

void TransferFunctionEditor::backupTensorRanks()
{
  globalRankBackup_  = *globalRank_;
  ranksBackup_       = *ranks_;
  ranksLastCommit_   = *ranks_;
}

void TransferFunctionEditor::restoreTensorRanks()
{
  *globalRank_  = globalRankBackup_;
  *ranks_       = ranksBackup_;
}


void TransferFunctionEditor::setTransferFunction(TransferFunctionPair *_transferFunction)
{
  transferFunction_ = _transferFunction;
  backupTransferFunction();
  transferFunctionGraph_->setTransferFunction(transferFunction_);
  update();
  transferFunctionGraph_->update();
}

void TransferFunctionEditor::setChannel(Channel _channel, bool _enabled)
{
  switch (_channel)
  {
  case TransferFunctionGraphCore::R:
    ui_.checkBoxR_->setChecked(_enabled);
    break;
  case TransferFunctionGraphCore::G:
    ui_.checkBoxG_->setChecked(_enabled);
    break;
  case TransferFunctionGraphCore::B:
    ui_.checkBoxB_->setChecked(_enabled);
    break;
  case TransferFunctionGraphCore::A:
    ui_.checkBoxA_->setChecked(_enabled);
    break;
  case TransferFunctionGraphCore::S:
    ui_.checkBoxS_->setChecked(_enabled);
    break;
  case TransferFunctionGraphCore::D:
    ui_.checkBoxD_->setChecked(_enabled);
    break;
  case TransferFunctionGraphCore::AM:
    ui_.checkBoxAm_->setChecked(_enabled);
    break;
  }
}

void TransferFunctionEditor::update()
{
  if (transferFunction_ != 0)
  {
    if (transferFunction_->first.size() != 0)
    {
      ui_.checkBoxR_->setCheckable(true);
      ui_.checkBoxG_->setCheckable(true);
      ui_.checkBoxB_->setCheckable(true);
      ui_.checkBoxA_->setCheckable(true);
    }
    else
    {
      ui_.checkBoxR_->setCheckState(Qt::Unchecked);
      ui_.checkBoxR_->setCheckable(false);
      ui_.checkBoxG_->setCheckState(Qt::Unchecked);
      ui_.checkBoxG_->setCheckable(false);
      ui_.checkBoxB_->setCheckState(Qt::Unchecked);
      ui_.checkBoxB_->setCheckable(false);
      ui_.checkBoxA_->setCheckState(Qt::Unchecked);
      ui_.checkBoxA_->setCheckable(false);
    }
    if (transferFunction_->second.size() != 0)
    {
      ui_.checkBoxS_->setCheckable(true);
      ui_.checkBoxD_->setCheckable(true);
      ui_.checkBoxAm_->setCheckable(true);
    }
    else
    {
      ui_.checkBoxS_->setCheckState(Qt::Unchecked);
      ui_.checkBoxS_->setCheckable(false);
      ui_.checkBoxD_->setCheckState(Qt::Unchecked);
      ui_.checkBoxD_->setCheckable(false);
      ui_.checkBoxAm_->setCheckState(Qt::Unchecked);
      ui_.checkBoxAm_->setCheckable(false);
    }
  }
  QWidget::update();
}

void TransferFunctionEditor::run()
{
  move(position_);
  show();
  raise();
  activateWindow();
}

void TransferFunctionEditor::accept()
{
  position_ = pos();
  QDialog::accept();
  if (transferFunction_ != 0)
  {
    backupTransferFunction();
    mainWnd_->updateTransferFunction();
    triggerMainWndUpdate();
  }
  if( globalRank_ != 0 )
  {
    backupTensorRanks();
    updateTensorRanks( false );
  }
}

void TransferFunctionEditor::reject()
{
  position_ = pos();
  QDialog::reject();
  if (transferFunction_ != 0)
  {
    restoreTransferFunction();
    mainWnd_->updateTransferFunction();
    triggerMainWndUpdate();
  }
  if (globalRank_ != 0 )
  {
    restoreTensorRanks();
    updateTensorRanks( false );
  }
}

void TransferFunctionEditor::updateCurve(bool _immediate)
{
  if (transferFunction_ != 0 && ui_.previewCheckBox_->isChecked())
  {
    // use a timer to avoid flooding the renderer with update requests
    timer_.stop();
    // limit the update frequency to half of the average framerate
///    if (_immediate || timer_.getElapsedTime() > 2.0 * ivsWidget_->getAverageTime2Render())
    if (_immediate || timer_.getElapsedTime() > 0.5 )
    {
      mainWnd_->updateTransferFunction();
      triggerMainWndUpdate();
      timer_.start();
    }
  }
}

void TransferFunctionEditor::changePreview(bool _checked)
{
  if (_checked && transferFunction_ != 0)
  {
    mainWnd_->updateTransferFunction();
    triggerMainWndUpdate();
  }
}

void TransferFunctionEditor::changeRangeAutoUpdate(bool)/// _checked)
{
///  ivsWidget_->setUpdateTransferFunctionRange(_checked);
}

void TransferFunctionEditor::closeEvent(QCloseEvent *_event)
{
  position_ = pos();
  QDialog::closeEvent(_event);
}

void TransferFunctionEditor::changeRenderer()
{
/*  IRaycasting *r = dynamic_cast<IRaycasting *>(ivsWidget_->getRenderer());
  ui_.transferFunctionTypeComboBox_->setEnabled(r != 0 ? true : false);
  if (r != 0)
    ui_.transferFunctionTypeComboBox_->setCurrentIndex(r->getClassification());
*/
}

void TransferFunctionEditor::changeGraph(int _i)
{
  delete transferFunctionGraph_;
  switch (_i)
  {
  case 0:
  {
    transferFunctionGraph_ = new FreehandGraph();
    break;
  }
  case 1:
  {
    transferFunctionGraph_ = new AnchorGraph(AnchorGraph::AnchorGraphCoreP(new RampGraphCore()));
    break;
  }
  case 2:
  {
    transferFunctionGraph_ = new AnchorGraph(AnchorGraph::AnchorGraphCoreP(new GaussGraphCore()));
    break;
  }
  case 3:
  {
    transferFunctionGraph_ = new AnchorGraph(AnchorGraph::AnchorGraphCoreP(new DoubleGaussGraphCore()));
    break;
  }
  case 4:
  {
    transferFunctionGraph_ = new AnchorGraph(AnchorGraph::AnchorGraphCoreP(new RampGaussGraphCore()));
    break;
  }
  }
  initializeGraph(transferFunctionGraph_);
}

void TransferFunctionEditor::changeTransferFunctionType(int)/// _i)
{
/*///  IRaycasting *r = dynamic_cast<IRaycasting *>(ivsWidget_->getRenderer());
  if (r != 0)
  {
    r->setClassification(static_cast<IRaycasting::Classification>(_i));
    triggerMainWndUpdate();
  }*/
}

void TransferFunctionEditor::loadTransferFunction(bool)
{
  if( transferFunction_ == 0 )
    return;

  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setViewMode(QFileDialog::Detail);
  dialog.setNameFilter("Transfer Function File (*.tf)");
  dialog.setDirectory(directory_);
  if (dialog.exec())
  {
    directory_ = dialog.directory();
    QStringList fileNames = dialog.selectedFiles();
    if (fileNames.begin() == fileNames.end())
      return;
    io::TransferFunctionFile transferFunctionFile(fileNames.begin()->toStdString());
    try
    {
      TransferFunctionFactory::ItemVector tfData;
      // try to read the data from the transfer function
      if (transferFunctionFile.exists())
      {
        transferFunctionFile.open();
        tfData = transferFunctionFile.readTransferFunction();
        transferFunctionFile.close();
        float *rgba = transferFunction_->first.getData();
        float *sda  = transferFunction_->second.getData();
        if ((tfData.size() == transferFunction_->first.size()) &&
            (tfData.size() == transferFunction_->second.size())  &&
            (rgba != 0) && (sda != 0))
        {
          TransferFunctionFactory::createRgbaSdaData(tfData, rgba, sda);
          mainWnd_->updateTransferFunction();
          triggerMainWndUpdate();
          update();
          transferFunctionGraph_->update();
        }
      }
    }
    catch (string &)
    {
      transferFunctionFile.close();
    }
  }
}

void TransferFunctionEditor::saveTransferFunction(bool)
{
  if( transferFunction_ == 0 )
    return;

  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setViewMode(QFileDialog::Detail);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setConfirmOverwrite(true);
  dialog.setNameFilter("Transfer Function File (*.tf)");
  dialog.setDirectory(directory_);
  if (dialog.exec())
  {
    directory_ = dialog.directory();
    QStringList fileNames = dialog.selectedFiles();
    if (fileNames.begin() == fileNames.end())
      return;
    io::TransferFunctionFile transferFunctionFile(fileNames.begin()->toStdString(), io::File::WRITE);
    try
    {
      float *rgba = transferFunction_->first.getData();
      float *sda  = transferFunction_->second.getData();
      unsigned int fSize = transferFunction_->first.size();
      if ((fSize > 0)                                     &&
          (fSize == transferFunction_->second.size()) &&
          (rgba != 0) && (sda != 0))
      {
        TransferFunctionFactory::ItemVector tfData;
        tfData = TransferFunctionFactory::createItemVector(fSize, rgba, sda);
        transferFunctionFile.open();
        transferFunctionFile.writeTransferFunction(tfData);
        transferFunctionFile.close();
      }
    }
    catch (string &)
    {
      transferFunctionFile.close();
    }
  }
}


void TransferFunctionEditor::initializeGraph(TransferFunctionGraph *_transferFunctionGraph)
{
  _transferFunctionGraph->setStateR(ui_.checkBoxR_->isChecked());
  _transferFunctionGraph->setStateG(ui_.checkBoxG_->isChecked());
  _transferFunctionGraph->setStateB(ui_.checkBoxB_->isChecked());
  _transferFunctionGraph->setStateA(ui_.checkBoxA_->isChecked());
  _transferFunctionGraph->setStateD(ui_.checkBoxD_->isChecked());
  _transferFunctionGraph->setStateS(ui_.checkBoxS_->isChecked());
  _transferFunctionGraph->setStateAm(ui_.checkBoxAm_->isChecked());
  ui_.gridLayout2_->addWidget(_transferFunctionGraph, 0, 0, 0, 0);

  connect(ui_.checkBoxR_, SIGNAL(toggled(bool)), _transferFunctionGraph, SLOT(setStateR(bool)));
  connect(ui_.checkBoxG_, SIGNAL(toggled(bool)), _transferFunctionGraph, SLOT(setStateG(bool)));
  connect(ui_.checkBoxB_, SIGNAL(toggled(bool)), _transferFunctionGraph, SLOT(setStateB(bool)));
  connect(ui_.checkBoxA_, SIGNAL(toggled(bool)), _transferFunctionGraph, SLOT(setStateA(bool)));
  connect(ui_.checkBoxS_, SIGNAL(toggled(bool)), _transferFunctionGraph, SLOT(setStateS(bool)));
  connect(ui_.checkBoxD_, SIGNAL(toggled(bool)), _transferFunctionGraph, SLOT(setStateD(bool)));
  connect(ui_.checkBoxAm_, SIGNAL(toggled(bool)), _transferFunctionGraph, SLOT(setStateAm(bool)));
  connect(_transferFunctionGraph, SIGNAL(curveUpdate(bool)), this, SLOT(updateCurve(bool)));

  if (histogram_)
    _transferFunctionGraph->setHistogram(histogram_, histogramSize_);

  if (transferFunction_)
  {
    _transferFunctionGraph->setTransferFunction(transferFunction_);
    update();
    _transferFunctionGraph->initialize();
    _transferFunctionGraph->update();
  }
}

void TransferFunctionEditor::backup(unsigned int _size,
                                    const float *_data,
                                    unsigned int &_backupSize,
                                    float *&_backupData,
                                    unsigned int _components)
{
  if (_data == 0 || _size != _backupSize)
  {
    delete [] _backupData;
    if (_size > 0)
      _backupData = new float[_components * _size];
    else
      _backupData = 0;
  }
  if (_size > 0)
    memcpy(_backupData, _data, _components * _size * sizeof(float));
  _backupSize = _size;
}

void TransferFunctionEditor::restore(unsigned int _backupSize,
                                     const float *_backupData,
                                     unsigned int _size,
                                     float *_data,
                                     unsigned int _components)
{
  // pre-condition: the arrays have the same size
  if (_size == _backupSize)
    memcpy(_data, _backupData, _components * _size * sizeof(float));
}

void TransferFunctionEditor::backupTransferFunction()
{
  if( transferFunction_ == 0 )
    return;

  backup(transferFunction_->first.size(),
         transferFunction_->first.getData(),
         rgbaSize_,
         rgba_,
         4);
  backup(transferFunction_->second.size(),
         transferFunction_->second.getData(),
         sdaSize_,
         sda_,
         3);
}

void TransferFunctionEditor::restoreTransferFunction()
{
  if( transferFunction_ == 0 )
    return;

  restore(rgbaSize_,
          rgba_,
          transferFunction_->first.size(),
          transferFunction_->first.getData(),
          4);
  restore(sdaSize_,
          sda_,
          transferFunction_->second.size(),
          transferFunction_->second.getData(),
          3);
}

void TransferFunctionEditor::triggerMainWndUpdate()
{
//    mainWnd_->updateTransferFunction();
///  ivsWidget_->updateTransferFunction();
///  ivsWidget_->update();
}

}
