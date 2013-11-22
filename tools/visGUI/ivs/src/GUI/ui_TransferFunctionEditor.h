/********************************************************************************
** Form generated from reading UI file 'TransferFunctionEditor.ui'
**
** Created
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRANSFERFUNCTIONEDITOR_H
#define UI_TRANSFERFUNCTIONEDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_TransferFunctionEditor
{
public:
    QGridLayout *gridLayout_;
    QPushButton *previewButton_;
    QDialogButtonBox *buttonBox_;
    QHBoxLayout *horizontalLayout5_;
    QPushButton *loadButton_;
    QPushButton *saveButton_;
    QHBoxLayout *horizontalLayout_;
    QFrame *line;
    QVBoxLayout *verticalLayout_;
    QLabel *editModeLabel_;
    QComboBox *graphComboBox_;
    QLabel *label;
    QComboBox *transferFunctionTypeComboBox_;
    QGroupBox *colorGroupBox_;
    QVBoxLayout *verticalLayout2_;
    QHBoxLayout *horizontalLayout3_;
    QCheckBox *checkBoxR_;
    QCheckBox *checkBoxG_;
    QCheckBox *checkBoxB_;
    QCheckBox *checkBoxA_;
    QGroupBox *materialGroupBox_;
    QVBoxLayout *verticalLayout3_;
    QHBoxLayout *horizontalLayout4_;
    QCheckBox *checkBoxS_;
    QCheckBox *checkBoxD_;
    QCheckBox *checkBoxAm_;
    QSpacerItem *horizontalSpacer_;
    QCheckBox *previewCheckBox_;
    QCheckBox *rangeAutoUpdateCheckBox_;
    QSpacerItem *verticalSpacer_;
    QVBoxLayout *verticalLayout;
    QLabel *label_13;
    QHBoxLayout *horizontalLayout;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_14;
    QSpinBox *spinBoxRankGlobal_;
    QGridLayout *gridLayoutTensor_;
    QLabel *label_6;
    QSpinBox *spinBoxRank8_;
    QLabel *label_5;
    QSpinBox *spinBoxRank7_;
    QSpinBox *spinBoxRank1_;
    QSpinBox *spinBoxRank3_;
    QLabel *label_4;
    QSpinBox *spinBoxRank6_;
    QSpinBox *spinBoxRank4_;
    QSpinBox *spinBoxRank5_;
    QSpinBox *spinBoxRank9_;
    QLabel *label_3;
    QLabel *label_7;
    QSpinBox *spinBoxRank2_;
    QLabel *label_9;
    QLabel *label_12;
    QLabel *label_10;
    QLabel *label_11;
    QLabel *label_8;
    QSpinBox *spinBoxRank0_;
    QCheckBox *checkBoxTensorAutoUpdate_;
    QPushButton *pushButtonTensorUpdate_;
    QSpacerItem *verticalSpacer;
    QFrame *frame_;
    QGridLayout *gridLayout2_;

    void setupUi(QDialog *TransferFunctionEditor)
    {
        if (TransferFunctionEditor->objectName().isEmpty())
            TransferFunctionEditor->setObjectName(QString::fromUtf8("TransferFunctionEditor"));
        TransferFunctionEditor->setEnabled(true);
        TransferFunctionEditor->resize(703, 699);
        gridLayout_ = new QGridLayout(TransferFunctionEditor);
        gridLayout_->setObjectName(QString::fromUtf8("gridLayout_"));
        previewButton_ = new QPushButton(TransferFunctionEditor);
        previewButton_->setObjectName(QString::fromUtf8("previewButton_"));

        gridLayout_->addWidget(previewButton_, 5, 4, 1, 1);

        buttonBox_ = new QDialogButtonBox(TransferFunctionEditor);
        buttonBox_->setObjectName(QString::fromUtf8("buttonBox_"));
        buttonBox_->setOrientation(Qt::Horizontal);
        buttonBox_->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_->addWidget(buttonBox_, 5, 5, 1, 1);

        horizontalLayout5_ = new QHBoxLayout();
        horizontalLayout5_->setObjectName(QString::fromUtf8("horizontalLayout5_"));
        loadButton_ = new QPushButton(TransferFunctionEditor);
        loadButton_->setObjectName(QString::fromUtf8("loadButton_"));

        horizontalLayout5_->addWidget(loadButton_);

        saveButton_ = new QPushButton(TransferFunctionEditor);
        saveButton_->setObjectName(QString::fromUtf8("saveButton_"));

        horizontalLayout5_->addWidget(saveButton_);


        gridLayout_->addLayout(horizontalLayout5_, 5, 3, 1, 1);

        horizontalLayout_ = new QHBoxLayout();
        horizontalLayout_->setObjectName(QString::fromUtf8("horizontalLayout_"));
        line = new QFrame(TransferFunctionEditor);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout_->addWidget(line);

        verticalLayout_ = new QVBoxLayout();
        verticalLayout_->setObjectName(QString::fromUtf8("verticalLayout_"));
        editModeLabel_ = new QLabel(TransferFunctionEditor);
        editModeLabel_->setObjectName(QString::fromUtf8("editModeLabel_"));

        verticalLayout_->addWidget(editModeLabel_);

        graphComboBox_ = new QComboBox(TransferFunctionEditor);
        graphComboBox_->setObjectName(QString::fromUtf8("graphComboBox_"));

        verticalLayout_->addWidget(graphComboBox_);

        label = new QLabel(TransferFunctionEditor);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout_->addWidget(label);

        transferFunctionTypeComboBox_ = new QComboBox(TransferFunctionEditor);
        transferFunctionTypeComboBox_->setObjectName(QString::fromUtf8("transferFunctionTypeComboBox_"));
        transferFunctionTypeComboBox_->setEnabled(false);

        verticalLayout_->addWidget(transferFunctionTypeComboBox_);

        colorGroupBox_ = new QGroupBox(TransferFunctionEditor);
        colorGroupBox_->setObjectName(QString::fromUtf8("colorGroupBox_"));
        verticalLayout2_ = new QVBoxLayout(colorGroupBox_);
        verticalLayout2_->setObjectName(QString::fromUtf8("verticalLayout2_"));
        horizontalLayout3_ = new QHBoxLayout();
        horizontalLayout3_->setObjectName(QString::fromUtf8("horizontalLayout3_"));
        checkBoxR_ = new QCheckBox(colorGroupBox_);
        checkBoxR_->setObjectName(QString::fromUtf8("checkBoxR_"));
        checkBoxR_->setEnabled(true);
        checkBoxR_->setChecked(true);

        horizontalLayout3_->addWidget(checkBoxR_);

        checkBoxG_ = new QCheckBox(colorGroupBox_);
        checkBoxG_->setObjectName(QString::fromUtf8("checkBoxG_"));
        checkBoxG_->setChecked(true);

        horizontalLayout3_->addWidget(checkBoxG_);

        checkBoxB_ = new QCheckBox(colorGroupBox_);
        checkBoxB_->setObjectName(QString::fromUtf8("checkBoxB_"));
        checkBoxB_->setChecked(true);

        horizontalLayout3_->addWidget(checkBoxB_);

        checkBoxA_ = new QCheckBox(colorGroupBox_);
        checkBoxA_->setObjectName(QString::fromUtf8("checkBoxA_"));
        checkBoxA_->setChecked(true);

        horizontalLayout3_->addWidget(checkBoxA_);


        verticalLayout2_->addLayout(horizontalLayout3_);


        verticalLayout_->addWidget(colorGroupBox_);

        materialGroupBox_ = new QGroupBox(TransferFunctionEditor);
        materialGroupBox_->setObjectName(QString::fromUtf8("materialGroupBox_"));
        verticalLayout3_ = new QVBoxLayout(materialGroupBox_);
        verticalLayout3_->setObjectName(QString::fromUtf8("verticalLayout3_"));
        horizontalLayout4_ = new QHBoxLayout();
        horizontalLayout4_->setObjectName(QString::fromUtf8("horizontalLayout4_"));
        checkBoxS_ = new QCheckBox(materialGroupBox_);
        checkBoxS_->setObjectName(QString::fromUtf8("checkBoxS_"));

        horizontalLayout4_->addWidget(checkBoxS_);

        checkBoxD_ = new QCheckBox(materialGroupBox_);
        checkBoxD_->setObjectName(QString::fromUtf8("checkBoxD_"));

        horizontalLayout4_->addWidget(checkBoxD_);

        checkBoxAm_ = new QCheckBox(materialGroupBox_);
        checkBoxAm_->setObjectName(QString::fromUtf8("checkBoxAm_"));

        horizontalLayout4_->addWidget(checkBoxAm_);

        horizontalSpacer_ = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        horizontalLayout4_->addItem(horizontalSpacer_);


        verticalLayout3_->addLayout(horizontalLayout4_);


        verticalLayout_->addWidget(materialGroupBox_);

        previewCheckBox_ = new QCheckBox(TransferFunctionEditor);
        previewCheckBox_->setObjectName(QString::fromUtf8("previewCheckBox_"));
        previewCheckBox_->setChecked(true);

        verticalLayout_->addWidget(previewCheckBox_);

        rangeAutoUpdateCheckBox_ = new QCheckBox(TransferFunctionEditor);
        rangeAutoUpdateCheckBox_->setObjectName(QString::fromUtf8("rangeAutoUpdateCheckBox_"));
        rangeAutoUpdateCheckBox_->setEnabled(false);
        rangeAutoUpdateCheckBox_->setChecked(true);

        verticalLayout_->addWidget(rangeAutoUpdateCheckBox_);

        verticalSpacer_ = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_->addItem(verticalSpacer_);


        horizontalLayout_->addLayout(verticalLayout_);


        gridLayout_->addLayout(horizontalLayout_, 0, 3, 2, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_13 = new QLabel(TransferFunctionEditor);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        verticalLayout->addWidget(label_13);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_14 = new QLabel(TransferFunctionEditor);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label_14);

        spinBoxRankGlobal_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRankGlobal_->setObjectName(QString::fromUtf8("spinBoxRankGlobal_"));
        spinBoxRankGlobal_->setMinimum(2);
        spinBoxRankGlobal_->setMaximum(32);
        spinBoxRankGlobal_->setValue(32);

        horizontalLayout_4->addWidget(spinBoxRankGlobal_);


        verticalLayout->addLayout(horizontalLayout_4);

        gridLayoutTensor_ = new QGridLayout();
        gridLayoutTensor_->setObjectName(QString::fromUtf8("gridLayoutTensor_"));
        gridLayoutTensor_->setContentsMargins(0, -1, -1, -1);
        label_6 = new QLabel(TransferFunctionEditor);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_6, 3, 0, 1, 1);

        spinBoxRank8_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank8_->setObjectName(QString::fromUtf8("spinBoxRank8_"));
        spinBoxRank8_->setMinimum(2);
        spinBoxRank8_->setMaximum(32);
        spinBoxRank8_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank8_, 8, 1, 1, 1);

        label_5 = new QLabel(TransferFunctionEditor);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_5, 2, 0, 1, 1);

        spinBoxRank7_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank7_->setObjectName(QString::fromUtf8("spinBoxRank7_"));
        spinBoxRank7_->setMinimum(2);
        spinBoxRank7_->setMaximum(32);
        spinBoxRank7_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank7_, 7, 1, 1, 1);

        spinBoxRank1_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank1_->setObjectName(QString::fromUtf8("spinBoxRank1_"));
        spinBoxRank1_->setMinimum(2);
        spinBoxRank1_->setMaximum(32);
        spinBoxRank1_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank1_, 1, 1, 1, 1);

        spinBoxRank3_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank3_->setObjectName(QString::fromUtf8("spinBoxRank3_"));
        spinBoxRank3_->setMinimum(2);
        spinBoxRank3_->setMaximum(32);
        spinBoxRank3_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank3_, 3, 1, 1, 1);

        label_4 = new QLabel(TransferFunctionEditor);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_4, 1, 0, 1, 1);

        spinBoxRank6_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank6_->setObjectName(QString::fromUtf8("spinBoxRank6_"));
        spinBoxRank6_->setMinimum(2);
        spinBoxRank6_->setMaximum(32);
        spinBoxRank6_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank6_, 6, 1, 1, 1);

        spinBoxRank4_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank4_->setObjectName(QString::fromUtf8("spinBoxRank4_"));
        spinBoxRank4_->setMinimum(2);
        spinBoxRank4_->setMaximum(32);
        spinBoxRank4_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank4_, 4, 1, 1, 1);

        spinBoxRank5_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank5_->setObjectName(QString::fromUtf8("spinBoxRank5_"));
        spinBoxRank5_->setMinimum(2);
        spinBoxRank5_->setMaximum(32);
        spinBoxRank5_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank5_, 5, 1, 1, 1);

        spinBoxRank9_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank9_->setObjectName(QString::fromUtf8("spinBoxRank9_"));
        spinBoxRank9_->setMinimum(2);
        spinBoxRank9_->setMaximum(32);
        spinBoxRank9_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank9_, 9, 1, 1, 1);

        label_3 = new QLabel(TransferFunctionEditor);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_3, 0, 0, 1, 1);

        label_7 = new QLabel(TransferFunctionEditor);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_7, 4, 0, 1, 1);

        spinBoxRank2_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank2_->setObjectName(QString::fromUtf8("spinBoxRank2_"));
        spinBoxRank2_->setMinimum(2);
        spinBoxRank2_->setMaximum(32);
        spinBoxRank2_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank2_, 2, 1, 1, 1);

        label_9 = new QLabel(TransferFunctionEditor);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_9, 6, 0, 1, 1);

        label_12 = new QLabel(TransferFunctionEditor);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_12, 9, 0, 1, 1);

        label_10 = new QLabel(TransferFunctionEditor);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_10, 7, 0, 1, 1);

        label_11 = new QLabel(TransferFunctionEditor);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_11, 8, 0, 1, 1);

        label_8 = new QLabel(TransferFunctionEditor);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayoutTensor_->addWidget(label_8, 5, 0, 1, 1);

        spinBoxRank0_ = new QSpinBox(TransferFunctionEditor);
        spinBoxRank0_->setObjectName(QString::fromUtf8("spinBoxRank0_"));
        spinBoxRank0_->setMinimum(2);
        spinBoxRank0_->setMaximum(32);
        spinBoxRank0_->setValue(32);

        gridLayoutTensor_->addWidget(spinBoxRank0_, 0, 1, 1, 1);


        verticalLayout->addLayout(gridLayoutTensor_);

        checkBoxTensorAutoUpdate_ = new QCheckBox(TransferFunctionEditor);
        checkBoxTensorAutoUpdate_->setObjectName(QString::fromUtf8("checkBoxTensorAutoUpdate_"));
        checkBoxTensorAutoUpdate_->setChecked(true);

        verticalLayout->addWidget(checkBoxTensorAutoUpdate_);

        pushButtonTensorUpdate_ = new QPushButton(TransferFunctionEditor);
        pushButtonTensorUpdate_->setObjectName(QString::fromUtf8("pushButtonTensorUpdate_"));

        verticalLayout->addWidget(pushButtonTensorUpdate_);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        gridLayout_->addLayout(verticalLayout, 0, 0, 1, 1);

        frame_ = new QFrame(TransferFunctionEditor);
        frame_->setObjectName(QString::fromUtf8("frame_"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(frame_->sizePolicy().hasHeightForWidth());
        frame_->setSizePolicy(sizePolicy);
        frame_->setFrameShape(QFrame::StyledPanel);
        frame_->setFrameShadow(QFrame::Sunken);
        gridLayout2_ = new QGridLayout(frame_);
        gridLayout2_->setContentsMargins(0, 0, 0, 0);
        gridLayout2_->setObjectName(QString::fromUtf8("gridLayout2_"));

        gridLayout_->addWidget(frame_, 0, 4, 2, 2);

#ifndef QT_NO_SHORTCUT
        editModeLabel_->setBuddy(graphComboBox_);
        label->setBuddy(transferFunctionTypeComboBox_);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(graphComboBox_, transferFunctionTypeComboBox_);
        QWidget::setTabOrder(transferFunctionTypeComboBox_, checkBoxR_);
        QWidget::setTabOrder(checkBoxR_, checkBoxG_);
        QWidget::setTabOrder(checkBoxG_, checkBoxB_);
        QWidget::setTabOrder(checkBoxB_, checkBoxA_);
        QWidget::setTabOrder(checkBoxA_, checkBoxS_);
        QWidget::setTabOrder(checkBoxS_, checkBoxD_);
        QWidget::setTabOrder(checkBoxD_, checkBoxAm_);
        QWidget::setTabOrder(checkBoxAm_, previewCheckBox_);
        QWidget::setTabOrder(previewCheckBox_, loadButton_);
        QWidget::setTabOrder(loadButton_, saveButton_);
        QWidget::setTabOrder(saveButton_, buttonBox_);

        retranslateUi(TransferFunctionEditor);
        QObject::connect(buttonBox_, SIGNAL(accepted()), TransferFunctionEditor, SLOT(accept()));
        QObject::connect(buttonBox_, SIGNAL(rejected()), TransferFunctionEditor, SLOT(reject()));
        QObject::connect(previewCheckBox_, SIGNAL(toggled(bool)), TransferFunctionEditor, SLOT(changePreview(bool)));
        QObject::connect(loadButton_, SIGNAL(clicked(bool)), TransferFunctionEditor, SLOT(loadTransferFunction(bool)));
        QObject::connect(saveButton_, SIGNAL(clicked(bool)), TransferFunctionEditor, SLOT(saveTransferFunction(bool)));
        QObject::connect(graphComboBox_, SIGNAL(currentIndexChanged(int)), TransferFunctionEditor, SLOT(changeGraph(int)));
        QObject::connect(transferFunctionTypeComboBox_, SIGNAL(currentIndexChanged(int)), TransferFunctionEditor, SLOT(changeTransferFunctionType(int)));
        QObject::connect(rangeAutoUpdateCheckBox_, SIGNAL(toggled(bool)), TransferFunctionEditor, SLOT(changeRangeAutoUpdate(bool)));
        QObject::connect(previewButton_, SIGNAL(clicked()), TransferFunctionEditor, SLOT(triggerMainWndUpdate()));

        QMetaObject::connectSlotsByName(TransferFunctionEditor);
    } // setupUi

    void retranslateUi(QDialog *TransferFunctionEditor)
    {
        TransferFunctionEditor->setWindowTitle(QApplication::translate("TransferFunctionEditor", "Transfer Function Editor", 0, QApplication::UnicodeUTF8));
        previewButton_->setText(QApplication::translate("TransferFunctionEditor", "Preview", 0, QApplication::UnicodeUTF8));
        loadButton_->setText(QApplication::translate("TransferFunctionEditor", "Load...", 0, QApplication::UnicodeUTF8));
        saveButton_->setText(QApplication::translate("TransferFunctionEditor", "Save...", 0, QApplication::UnicodeUTF8));
        editModeLabel_->setText(QApplication::translate("TransferFunctionEditor", "Edit Mode:", 0, QApplication::UnicodeUTF8));
        graphComboBox_->clear();
        graphComboBox_->insertItems(0, QStringList()
         << QApplication::translate("TransferFunctionEditor", "Freehand", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TransferFunctionEditor", "Ramp", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TransferFunctionEditor", "Gauss", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TransferFunctionEditor", "Double Gauss", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TransferFunctionEditor", "Ramp Gauss", 0, QApplication::UnicodeUTF8)
        );
        label->setText(QApplication::translate("TransferFunctionEditor", "Transfer Function Type:", 0, QApplication::UnicodeUTF8));
        transferFunctionTypeComboBox_->clear();
        transferFunctionTypeComboBox_->insertItems(0, QStringList()
         << QApplication::translate("TransferFunctionEditor", "Voxel Value", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TransferFunctionEditor", "Derivative", 0, QApplication::UnicodeUTF8)
        );
        colorGroupBox_->setTitle(QApplication::translate("TransferFunctionEditor", "Color Channels", 0, QApplication::UnicodeUTF8));
        checkBoxR_->setText(QApplication::translate("TransferFunctionEditor", "R", 0, QApplication::UnicodeUTF8));
        checkBoxG_->setText(QApplication::translate("TransferFunctionEditor", "G", 0, QApplication::UnicodeUTF8));
        checkBoxB_->setText(QApplication::translate("TransferFunctionEditor", "B", 0, QApplication::UnicodeUTF8));
        checkBoxA_->setText(QApplication::translate("TransferFunctionEditor", "A", 0, QApplication::UnicodeUTF8));
        materialGroupBox_->setTitle(QApplication::translate("TransferFunctionEditor", "Material Properties", 0, QApplication::UnicodeUTF8));
        checkBoxS_->setText(QApplication::translate("TransferFunctionEditor", "S", 0, QApplication::UnicodeUTF8));
        checkBoxD_->setText(QApplication::translate("TransferFunctionEditor", "D", 0, QApplication::UnicodeUTF8));
        checkBoxAm_->setText(QApplication::translate("TransferFunctionEditor", "A", 0, QApplication::UnicodeUTF8));
        previewCheckBox_->setText(QApplication::translate("TransferFunctionEditor", "Preview", 0, QApplication::UnicodeUTF8));
        rangeAutoUpdateCheckBox_->setText(QApplication::translate("TransferFunctionEditor", "Range Auto Update", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("TransferFunctionEditor", "Tensor Ranks:", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("TransferFunctionEditor", "Global", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("TransferFunctionEditor", " Level 3", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("TransferFunctionEditor", " Level 2", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("TransferFunctionEditor", " Level 1", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("TransferFunctionEditor", " Level 0", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("TransferFunctionEditor", "Level 4", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("TransferFunctionEditor", "Level 6", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("TransferFunctionEditor", "Level 9", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("TransferFunctionEditor", "Level 7", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("TransferFunctionEditor", "Level 8", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("TransferFunctionEditor", "Level 5", 0, QApplication::UnicodeUTF8));
        checkBoxTensorAutoUpdate_->setText(QApplication::translate("TransferFunctionEditor", "Auto update", 0, QApplication::UnicodeUTF8));
        pushButtonTensorUpdate_->setText(QApplication::translate("TransferFunctionEditor", "Update", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TransferFunctionEditor: public Ui_TransferFunctionEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRANSFERFUNCTIONEDITOR_H
