#include <QThread>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include <QRegularExpression>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Command.h"
#include <windows.h>
#include "mcp2221_dll_um.h"
#include "GetErrName.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "LogManager.h"
#include "LogLevel.h"
#include <qtimer.h>
#include "SleepGuard.h"
#include "SensorCard.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

//#define PID 0xDD
//#define VID 0x4D8

#define FILE_APP_VERSION_INFO 0xF900

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->setFixedSize(1300,700);
    ui->setupUi(this);

    AppVersion=windowTitle();



    m_sht4xCard =
                new SensorCard(
                    "SHT4x",
                    "°C",
                    "%RH",
                    this);

        m_dsp368Card =
                new SensorCard(
                    "DSP368",
                    "°C",
                    "Pa",
                    this);

        m_deviceidCard=
                new SensorCard(
                    "Device ID",
                    "",
                    "",
                    this);

    ui->horizontalLayout->addWidget(m_sht4xCard);
    ui->horizontalLayout->addWidget(m_dsp368Card);
    ui->horizontalLayout->addWidget(m_deviceidCard);


    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    checkComPortIsMcp2221 = settings.value("USB_HUB/Check_COM_Port_Is_MCP2221", "0").toInt();
    Auto_Connect_Serial_Number = settings.value("USB_HUB/Auto_Connect_Serial_Number", "").toString();
    Auto_Connect_VID = settings.value("USB_HUB/Auto_Connect_VID", "").toString();
    Auto_Connect_PID = settings.value("USB_HUB/Auto_Connect_PID", "").toString();
    qDebug() << "[MainWindow]Auto_Connect_VID:" << Auto_Connect_VID;
    qDebug() << "[MainWindow]Auto_Connect_PID:" << Auto_Connect_PID;
    if(!Auto_Connect_VID.isEmpty()&&!Auto_Connect_PID.isEmpty())
    {
        bAutoConnect=true;
        qDebug() << "[MainWindow]Auto_Connect VID&PID not empty";
    }
    else
    {
        bAutoConnect=false;
        qDebug() << "[MainWindow]Auto_Connect VID&PID is empty";
    }
    needRecordInfo = settings.value("INFO_Record/Enable", "0").toInt();

    needAutoRefreshWhenConnect = settings.value("Connect_Auto_Refresh/Enable", "0").toInt();

    if(needAutoRefreshWhenConnect>1)
        needAutoRefreshWhenConnect=1;

    autoRefreshSource  = settings.value("Connect_Auto_Refresh/Refresh_Source", "0").toInt();

    if(autoRefreshSource>1)
        autoRefreshSource=1;

    csvAutoImport = settings.value("Init_CSV_Auto_Import/Enable", "0").toInt();

    sht4xCommandIndex=settings.value("SHT4x/CommandType", "0").toInt();
    readoutIntervalIndex=settings.value("SHT4x/Readout_Interval_Index", "0").toInt();
    readoutInterval=getSHT4xReadOutInterval(readoutIntervalIndex);
    sht4xEnable=settings.value("SHT4x/SHT4x_Enable", "1").toInt();
    sht4xEnableLog=settings.value("SHT4x/SHT4x_Enable_Log", "1").toInt();
    heatingProtectionTemperature=settings.value("SHT4x/Heating_protection_temperature", "65.0").toFloat();
    sht4xLogType=settings.value("SHT4x/SHT4x_Base_TEMP_And_Heating_TEMP_Log_Separate", "0").toFloat();
    enableCal=settings.value("SHT4x/Enable_CAL", "0").toFloat();

    dsp368TempOSRIndex=settings.value("DSP368/TEMP_OVERSAMPLE_Type", "0").toInt();
    dsp368PressureOSRIndex=settings.value("DSP368/PRESSURE_OVERSAMPLE_Type", "0").toInt();
    dsp368ReadoutIntervalIndex=settings.value("DSP368/Readout_Interval_Index", "2").toInt();
    dsp368ReadoutInterval=getDPS368ReadOutInterval(dsp368ReadoutIntervalIndex);
    dsp368Enable=settings.value("DSP368/DSP368_Enable", "1").toInt();
    dsp368EnableLog=settings.value("DSP368/DSP368_Enable_Log", "1").toInt();

    if(settings.contains("SMB100/Read_From_Source"))
    {
        readFromIndex=settings.value("SMB100/Read_From_Source", "0").toInt();
        sbm100_breadFrom=true;

    }
    else
    {
        sbm100_breadFrom=false;
        readFromIndex=0;
    }

    if(settings.contains("SMB100/Write_To_Source"))
    {
        writeToIndex=settings.value("SMB100/Write_To_Source", "1").toInt();
        sbm100_bwriteTo=true;
    }
    else
    {
        sbm100_bwriteTo=false;
        writeToIndex=1;
    }

    burnInfo= settings.value("BURN_INFO/Enable", "0").toInt();
    C1ModelNumber = settings.value("BURN_INFO/C1ModelNumber", "").toString();
    C1VersionLetter = settings.value("BURN_INFO/C1VersionLetter", "").toString();
    C1SerialNumber = settings.value("BURN_INFO/C1SerialNumber", "").toString();
    C1Sensitivity = settings.value("BURN_INFO/C1Sensitivity", "").toString();
    C1ReferenceFrequency = settings.value("BURN_INFO/C1ReferenceFrequency", "").toString();
    C1UnitsCode = settings.value("BURN_INFO/C1UnitsCode", "").toString();
    C1FrequencyRangeMin = settings.value("BURN_INFO/C1FrequencyRangeMin", "").toString();
    C1FrequencyRangeMax = settings.value("BURN_INFO/C1FrequencyRangeMax", "").toString();
    C1ChannelAssignment = settings.value("BURN_INFO/C1ChannelAssignment", "").toString();

    C2ModelNumber = settings.value("BURN_INFO/C2ModelNumber", "").toString();
    C2VersionLetter = settings.value("BURN_INFO/C2VersionLetter", "").toString();
    C2SerialNumber = settings.value("BURN_INFO/C2SerialNumber", "").toString();
    C2Sensitivity = settings.value("BURN_INFO/C2Sensitivity", "").toString();
    C2ReferenceFrequency = settings.value("BURN_INFO/C2ReferenceFrequency", "").toString();
    C2UnitsCode = settings.value("BURN_INFO/C2UnitsCode", "").toString();
    C2FrequencyRangeMin = settings.value("BURN_INFO/C2FrequencyRangeMin", "").toString();
    C2FrequencyRangeMax = settings.value("BURN_INFO/C2FrequencyRangeMax", "").toString();
    C2ChannelAssignment = settings.value("BURN_INFO/C2ChannelAssignment", "").toString();

    systemDigitInterfaceType = settings.value("BURN_INFO/systemDigitInterfaceType", "").toString();
    systemBitClockFrequency = settings.value("BURN_INFO/systemBitClockFrequency", "").toString();
    systemWordLength = settings.value("BURN_INFO/systemWordLength", "").toString();
    systemSampleRate = settings.value("BURN_INFO/systemSampleRate", "").toString();
    systemSerialnumber = settings.value("BURN_INFO/systemSerialnumber", "").toString();
    systemSensitivity = settings.value("BURN_INFO/systemSensitivity", "").toString();
    systemCalibrationDate = settings.value("BURN_INFO/systemCalibrationDate", "").toString();
    systemManufactuerID = settings.value("BURN_INFO/systemManufactuerID", "").toString();
    SerialNumberOffset= settings.value("BURN_INFO/SerialNumberOffser", "1").toInt();

    isDSP368Exist=false;
    isSBM100Exist=false;
    bSaveAllFlag=false;

    infoNoteEdit = nullptr;
    writeConfig = nullptr;
    clearBtn = nullptr;
    getTimeBtn = nullptr;
    singleReadBtn = nullptr;
    setSHT4xSoftrestBtn = nullptr;
    readSHT4xSerialNumberBtn = nullptr;
    tempCalLineEdit= nullptr;
    humidityCalLineEdit= nullptr;
    saveCalBtn = nullptr;
    readCalBtn = nullptr;
    readFromBtn = nullptr;
    writeToBtn = nullptr;
    startCheckBox = nullptr;
    enableLogCheckBox = nullptr;
    baseTemp= nullptr;
    baseHumidity= nullptr;
    heatingTemp= nullptr;
    heatingHumidity= nullptr;
    for(int i=0;i<2;i++)
    {
        infoSHT4xComboBox[i] = nullptr;
    }

    for(int i=0;i<3;i++)
    {
      infoDPS368ComboBox[i] = nullptr;

    }

    for(int i=0;i<9;i++)
    {
        infoSBM100ComboBox[i] = nullptr;
    }

    for(int i=0;i<5;i++)
    {
        infoSBM100LineEdit[i]= nullptr;
    }
    comboBox = nullptr;
    slider = nullptr;
    infoSBM100slider = nullptr;

    for(int i=0;i<10;i++)
    {
        GroupBox[i] = nullptr;
        btnGrup[i] = nullptr;
    }

    for(int i=0;i<40;i++)
    {
        infoLineEdit[i]= nullptr;
        radioButton[i]= nullptr;
        descriptionLabel[i]= nullptr;
    }

    //bRefreshInfoBlock1=false;
    //bRefreshInfoBlock2=false;

    for(int i=0;i<4;i++)
        ui->uartCH_comboBox->addItem(QLocale().toString(i));

    mSerial = new QSerialPort(this);

    timer = new QTimer(this);

    timer->setTimerType(Qt::PreciseTimer);   // 高精度
    timer->setInterval(TIMER_INTERVAL);                  // 10 ms

    //mSerialScanTimer = new QTimer(this);
    //mSerialScanTimer->setInterval(1000);
    //mSerialScanTimer->start();

    memset(&info,0x00,sizeof(DeviceInfo));
    memset(&note,0x00,sizeof(DeviceInfoNote));

    CMD_INDEX = 0;
    TEMP_DATA_BUF_INDEX=0;
    DATA_BUF_INDEX=0;
    bUpgrade=false;
    comPortDisconnect();

    tempCal=0;
    humidityCal=0;

    dcb_cutoff=0;

    initInfoRadioButton();
    initPage0();
    initPage2();
    initPage3();
    initPage4();

    if(csvAutoImport)
        autoImportCSVFile();

    if (workerThread) {
        workerThread->stop();
        workerThread->wait();
        delete workerThread;
    }

    workerThread = new CheckWorkerThread(this);

    ui->page0ReadMethod_comboBox->setCurrentIndex(autoRefreshSource);
    ui->page2ReadMethod_comboBox->setCurrentIndex(autoRefreshSource);
    ui->page3ReadMethod_comboBox->setCurrentIndex(autoRefreshSource);
    ui->page4ReadMethod_comboBox->setCurrentIndex(autoRefreshSource);

    /*ui->tabWidget->setStyleSheet("QTabWidget#tabWidget{background-color:rgb(222,222,222);}\
                                 QTabBar::tab{min-width:62px;margin-left:2px;margin-right:2px;background-color:rgb(222,222,222);color:rgb(0,0,0);font-weight: bold;font:10pt;}\
                                 QTabWidget::pane {border: 1px solid #ffffff;}\
                                 QTabBar::tab::selected{background-color:rgb(255,255,255);color:rgb(0,0,0);font-weight: bold;font:10pt}");*/
    ui->tabWidget->setCurrentIndex(0);


    ui->tableWidget_page0->setAlternatingRowColors(true);
    ui->tableWidget_page0->setColumnWidth(0,80);
    ui->tableWidget_page0->setColumnWidth(1,130);
    ui->tableWidget_page0->setStyleSheet("QTableWidget {"
                                         "background-color: #4A4A4A;"
                                         "  gridline-color: #FFFFFF;"
                                         "  border: 1px solid white;"
                                         "}"
                                         "QHeaderView::section {"
                                         "background-color: #4A4A4A;"
                                         "color: white;"
                                         "border: 0px solid white;"
                                         "}"
                                         "QTableCornerButton::section {"
                                         "  background-color: #4A4A4A;"
                                         "}"
                                         "QTableWidget::item {"
                                         "  padding: 1px;"
                                         "  background-color: #000000;"
                                         "  border: 1px solid #FFFFFF;"
                                         "}"
                                         "QTableWidget::item:selected {"
                                         "  background-color: #007ae6;"
                                         "}");

    ui->tableWidget_page0->verticalHeader()
        ->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidget_page2->setAlternatingRowColors(true);
    ui->tableWidget_page2->setColumnWidth(0,80);
    ui->tableWidget_page2->setColumnWidth(1,130);

    ui->tableWidget_page2->setStyleSheet("QTableWidget {"
                                         "background-color: #4A4A4A;"
                                         "  gridline-color: #FFFFFF;"
                                         "  border: 1px solid white;"
                                         "}"
                                         "QHeaderView::section {"
                                         "background-color: #4A4A4A;"
                                         "color: white;"
                                         "border: 0px solid white;"
                                         "}"
                                         "QTableCornerButton::section {"
                                         "  background-color: #4A4A4A;"
                                         "}"
                                         "QTableWidget::item {"
                                         "  padding: 1px;"
                                         "  background-color: #000000;"
                                         "  border: 1px solid #FFFFFF;"
                                         "}"
                                         "QTableWidget::item:selected {"
                                         "  background-color: #007ae6;"
                                         "}");

    ui->tableWidget_page2->verticalHeader()
        ->setDefaultAlignment(Qt::AlignCenter);


    ui->tableWidget_page3->setAlternatingRowColors(true);
    ui->tableWidget_page3->setColumnWidth(0,80);
    ui->tableWidget_page3->setColumnWidth(1,130);

    ui->tableWidget_page3->setStyleSheet("QTableWidget {"
                                         "background-color: #4A4A4A;"
                                         "  gridline-color: #FFFFFF;"
                                         "  border: 1px solid white;"
                                         "}"
                                         "QHeaderView::section {"
                                         "background-color: #4A4A4A;"
                                         "color: white;"
                                         "border: 0px solid white;"
                                         "}"
                                         "QTableCornerButton::section {"
                                         "  background-color: #4A4A4A;"
                                         "}"
                                         "QTableWidget::item {"
                                         "  padding: 1px;"
                                         "  background-color: #000000;"
                                         "  border: 1px solid #FFFFFF;"
                                         "}"
                                         "QTableWidget::item:selected {"
                                         "  background-color: #007ae6;"
                                         "}");

    ui->tableWidget_page3->verticalHeader()
        ->setDefaultAlignment(Qt::AlignCenter);



    ui->tableWidget_page4->setAlternatingRowColors(true);
    ui->tableWidget_page4->setColumnWidth(0,80);
    ui->tableWidget_page4->setColumnWidth(1,130);

    ui->tableWidget_page4->setStyleSheet("QTableWidget {"
                                         "background-color: #4A4A4A;"
                                         "  gridline-color: #FFFFFF;"
                                         "  border: 1px solid white;"
                                         "}"
                                         "QHeaderView::section {"
                                         "background-color: #4A4A4A;"
                                         "color: white;"
                                         "border: 0px solid white;"
                                         "}"
                                         "QTableCornerButton::section {"
                                         "  background-color: #4A4A4A;"
                                         "}"
                                         "QTableWidget::item {"
                                         "  padding: 1px;"
                                         "  background-color: #000000;"
                                         "  border: 1px solid #FFFFFF;"
                                         "}"
                                         "QTableWidget::item:selected {"
                                         "  background-color: #007ae6;"
                                         "}");

    ui->tableWidget_page4->verticalHeader()
        ->setDefaultAlignment(Qt::AlignCenter);

    ui->page2WriteRegBtn->setVisible(false);
    ui->page3WriteRegBtn->setVisible(false);
    ui->page4WriteRegBtn->setVisible(false);
    //ui->deviceIDlabel->setVisible(false);
    //connect(mSerialScanTimer, &QTimer::timeout,this, &MainWindow::updateSerialPorts);
    connect(mSerial, &QSerialPort::readyRead,this, &MainWindow::serialReadyRead);
    connect(ui->uartCH_comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(uartChannelChange(int)));
    connect(ui->connectBtn,SIGNAL(clicked()),this,SLOT(connectBtn_clicked()));
    connect(ui->disconnectBtn,SIGNAL(clicked()),this,SLOT(disconnectBtn_clicked()));
    connect(ui->selectFileBtn,SIGNAL(clicked()),this,SLOT(openFileBtn_clicked()));

    connect(ui->upgradeBtn,SIGNAL(clicked()),this,SLOT(upgradeBtn_clicked()));
    connect(ui->page0RefreshBtn,SIGNAL(clicked()),this,SLOT(page0RefreshBtn_clicked()));
    connect(ui->page0SaveBtn,SIGNAL(clicked()),this,SLOT(page0SaveBtn_clicked()));
    connect(ui->page0SaveAllBtn,SIGNAL(clicked()),this,SLOT(page0SaveAllBtn_clicked()));
    connect(ui->page0ImportBtn,SIGNAL(clicked()),this,SLOT(page0ImportBtn_clicked()));
    connect(ui->page0ExportBtn,SIGNAL(clicked()),this,SLOT(page0ExportBtn_clicked()));
    connect(ui->page0WriteRegBtn,SIGNAL(clicked()),this,SLOT(page0WriteRegBtn_clicked()));
    connect(ui->page0WriteAllBtn,SIGNAL(clicked()),this,SLOT(page0WriteAllBtn_clicked()));

    connect(ui->page2RefreshBtn,SIGNAL(clicked()),this,SLOT(page2RefreshBtn_clicked()));
    connect(ui->page2SaveBtn,SIGNAL(clicked()),this,SLOT(page2SaveBtn_clicked()));
    connect(ui->page2SaveAllBtn,SIGNAL(clicked()),this,SLOT(page2SaveAllBtn_clicked()));
    connect(ui->page2ImportBtn,SIGNAL(clicked()),this,SLOT(page2ImportBtn_clicked()));
    connect(ui->page2ExportBtn,SIGNAL(clicked()),this,SLOT(page2ExportBtn_clicked()));
    connect(ui->page2WriteRegBtn,SIGNAL(clicked()),this,SLOT(page2WriteRegBtn_clicked()));
    connect(ui->page2WriteAllBtn,SIGNAL(clicked()),this,SLOT(page2WriteAllBtn_clicked()));

    connect(ui->page3RefreshBtn,SIGNAL(clicked()),this,SLOT(page3RefreshBtn_clicked()));
    connect(ui->page3SaveBtn,SIGNAL(clicked()),this,SLOT(page3SaveBtn_clicked()));
    connect(ui->page3SaveAllBtn,SIGNAL(clicked()),this,SLOT(page3SaveAllBtn_clicked()));
    connect(ui->page3ImportBtn,SIGNAL(clicked()),this,SLOT(page3ImportBtn_clicked()));
    connect(ui->page3ExportBtn,SIGNAL(clicked()),this,SLOT(page3ExportBtn_clicked()));
    connect(ui->page3WriteRegBtn,SIGNAL(clicked()),this,SLOT(page3WriteRegBtn_clicked()));
    connect(ui->page3WriteAllBtn,SIGNAL(clicked()),this,SLOT(page3WriteAllBtn_clicked()));

    connect(ui->page4RefreshBtn,SIGNAL(clicked()),this,SLOT(page4RefreshBtn_clicked()));
    connect(ui->page4SaveBtn,SIGNAL(clicked()),this,SLOT(page4SaveBtn_clicked()));
    connect(ui->page4SaveAllBtn,SIGNAL(clicked()),this,SLOT(page4SaveAllBtn_clicked()));
    connect(ui->page4ImportBtn,SIGNAL(clicked()),this,SLOT(page4ImportBtn_clicked()));
    connect(ui->page4ExportBtn,SIGNAL(clicked()),this,SLOT(page4ExportBtn_clicked()));
    connect(ui->page4WriteRegBtn,SIGNAL(clicked()),this,SLOT(page4WriteRegBtn_clicked()));
    connect(ui->page4WriteAllBtn,SIGNAL(clicked()),this,SLOT(page4WriteAllBtn_clicked()));


    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    //#if 0
    connect(ui->tableWidget_page0->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(tableWidgetPage0SelectChange(QItemSelection,QItemSelection)));
    //#endif
    connect(ui->tableWidget_page2->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(tableWidgetPage2SelectChange(QItemSelection,QItemSelection)));
    connect(ui->tableWidget_page3->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(tableWidgetPage3SelectChange(QItemSelection,QItemSelection)));
    connect(ui->tableWidget_page4->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(tableWidgetPage4SelectChange(QItemSelection,QItemSelection)));

    connect(workerThread, &CheckWorkerThread::checkFunctionCalled, this, &MainWindow::onCheckFunctionCalled);
    connect(workerThread, &CheckWorkerThread::workFinished, this, &MainWindow::onWorkFinished);

    connect(timer, &QTimer::timeout, this, &MainWindow::onTimer);

    readOutIntervalCount=0;
    updateSerialPorts();

    workerThread->start();
    mSerialScanTimer.start();


    //mCheckOfflineTimer.start();
    //bSingleRead=false;

    ui->tableWidget_page4->setCurrentCell(1,0);
    ui->tableWidget_page3->setCurrentCell(1,0);
    ui->tableWidget_page2->setCurrentCell(1,0);
    ui->tableWidget_page0->setCurrentCell(1,0);

    //SleepGuard *sleepGuard = new SleepGuard(this);
    //sleepGuard->preventSleep();

    bCheckCMDSend=false;
    bWaitReadOutResponse=false;

}

void MainWindow::setReconnectFlag()
{
    reConnectSync=true;
    reConnectSyncCount=0;
    bDisconnect=false;
}

void MainWindow::resetCheckDisconnectFlag()
{
    //qDebug() << "[MainWindow]resetCheckDisconnectFlag";
    bCheckCMDSend=false;
    waitCMDResponseCount=0;
    SendCommandInterval=0;
    if(bDisconnect)
    {
        setReconnectFlag();
    }

}

void MainWindow::resetSaveAllTimeout()
{
    saveAllTimeOut=0;
}

void MainWindow::checkSaveAllTimeout()
{
    if(bSaveAllFlag)
    {
        saveAllTimeOut++;
        if(saveAllTimeOut>=300)
        {
            if(writeEEPROMprogress)
            {
                writeEEPROMprogress->close();

                delete writeEEPROMprogress;

                writeEEPROMprogress = nullptr;
            }
            resetSaveAllTimeout();
            bSaveAllFlag=false;
            qDebug() <<"[MainWindow]Save Timeout!!";
            QMessageBox::information(NULL, "MessageBox", "Save Timeout!!");
        }
    }
    else
    {
        resetSaveAllTimeout();
    }
}

void MainWindow::checkDisconnect()
{
    if(!IsConnect)
        return;

    if(reConnectSync)
    {
        reConnectSyncCount++;
        if(reConnectSyncCount>=20)
        {
            qDebug() << "[MainWindow]==========reSync==========";
            reConnectSync=false;
            reConnectSyncCount=0;
            syncDeviceData();
        }
        return;
    }

    SendCommandInterval++;
    if(SendCommandInterval>=100)
    {
        //qDebug() << "[MainWindow]checkDisconnect Send CMD";
        //qDebug() << "[MainWindow]==========GET_FW_VERSION_CMD==========";
        GenCommand(GET_FW_VERSION_CMD, NULL, 0);
        if(!bCheckCMDSend)
        {
            bCheckCMDSend=true;
            waitCMDResponseCount=0;
            SendCommandInterval=0;
        }
    }

    if(bCheckCMDSend)
    {
        if(!bDisconnect)
        {
            waitCMDResponseCount++;
            if(waitCMDResponseCount>=120)
            {
                //qDebug() << "[MainWindow]disconnect";
                bCheckCMDSend=false;
                waitCMDResponseCount=0;
                reConnectSync=false;
                bDisconnect=true;
                //ui->temperatureLabel->setText("Temperature:");
                //ui->humidtyLabel->setText("Humidty:");
                m_sht4xCard->clearValue1();
                m_sht4xCard->clearValue2();
                //ui->pressureLabel->setText("Pressure:");
                //ui->tempDPS368Label->setText("Temperature:");
                m_dsp368Card->clearValue1();
                m_dsp368Card->clearValue2();
                m_deviceidCard->clearDeviceId();
                //ui->deviceIDlabel->setText("");
                //setWindowTitle(AppVersion+" Device ID:");


                isDSP368Exist=false;
                isSBM100Exist=false;
                infoRadioButton[5]->setEnabled(false);
                infoRadioButton[6]->setEnabled(false);
                if(infoBtnGrup->checkedId()==5||infoBtnGrup->checkedId()==6)
                {
                    infoBtnGrup->button(0)->setChecked(true);

                    if(ui->tabWidget->currentIndex()==4)
                        onTabChanged(4);
                }
            }
        }
    }
}

void MainWindow::onTimer()
{
    if(!bUpgrade)
    {
        //QString logTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz 'ms'");
        //qDebug() << "[MainWindow]TIM:" << logTime;
        checkSaveAllTimeout();

        checkDisconnect();

        if(dsp368Enable)
        {

            dsp368ReadOutIntervalCount++;
            if(dsp368ReadOutIntervalCount>=dsp368ReadoutInterval)
            {

                if(!bWaitDsp368ReadOutResponse)
                {
                    readOutDSP368();
                    bWaitDsp368ReadOutResponse=true;
                    dsp368ReadOutIntervalCount=0;
                    waitDsp368ReadOutResponseCount=0;
                }
                else
                {
                    waitDsp368ReadOutResponseCount++;
                    if(waitDsp368ReadOutResponseCount>=80)
                    {
                        bWaitDsp368ReadOutResponse=false;
                        waitDsp368ReadOutResponseCount=0;
                    }

                }
            }

        }

        if(sht4xEnable)
        {

            readOutIntervalCount++;
            if(readOutIntervalCount>=readoutInterval)
            {

                if(!bWaitReadOutResponse)
                {
                    readOutSHT4x();
                    bWaitReadOutResponse=true;
                    readOutIntervalCount=0;
                    waitReadOutResponseCount=0;
                }
                else
                {
                    waitReadOutResponseCount++;
                    if(waitReadOutResponseCount>=80)
                    {
                        bWaitReadOutResponse=false;
                        waitReadOutResponseCount=0;
                    }

                }
            }

        }

    }
}

void MainWindow::initInfoRadioButton()
{
    qDebug() << "[MainWindow]initInfoRadioButton";
    QWidget* tabPage = ui->tabWidget->widget(4);

    infoBtnGrup = new QButtonGroup(tabPage);

    infoGroupBox = new QGroupBox(tabPage);
    infoGroupBox->setStyleSheet(QStringLiteral("QGroupBox{border:1px solid white;font-weight: bold;font-size: 10pt;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top left;padding:0.3px;}"));

    for(int i=0;i<7;i++)
    {
        infoRadioButton[i]= new QRadioButton(tabPage);
        infoRadioButton[i]->setStyleSheet(R"(
            QRadioButton {
                color: white;
                spacing: 8px;
            }
            QRadioButton::indicator {
                width: 16px;
                height: 16px;
            }
            QRadioButton::indicator:unchecked {
                image: url(:/radio_unchecked.ico);
            }
            QRadioButton::indicator:checked {
                image: url(:/radio_checked.ico);
            }
            QRadioButton::indicator:disabled {
                image: url(:/radio_disabled.ico);
            }
        )");

    }

    infoGroupBox->setGeometry(20,30,270,240);
    infoGroupBox->setTitle("Info Utility");
    infoGroupBox->show();

    infoRadioButton[0]->setGeometry(40,60,100,20);
    infoRadioButton[0]->setText("Show Info");
    infoRadioButton[0]->show();

    infoRadioButton[1]->setGeometry(40,90,100,20);
    infoRadioButton[1]->setText("Edit Info");
    infoRadioButton[1]->show();

    infoRadioButton[2]->setGeometry(40,120,100,20);
    infoRadioButton[2]->setText("Edit Note");
    infoRadioButton[2]->show();

    infoRadioButton[3]->setGeometry(40,150,100,20);
    infoRadioButton[3]->setText("Edit HUB Info");
    infoRadioButton[3]->show();

    infoRadioButton[4]->setGeometry(40,180,240,20);
    infoRadioButton[4]->setText("SHT4x setting");
    //infoRadioButton[4]->setEnabled(false);
    infoRadioButton[4]->show();

    infoRadioButton[5]->setGeometry(40,210,240,20);
    infoRadioButton[5]->setText("DSP368 setting");
    if(isDSP368Exist)
        infoRadioButton[5]->setEnabled(true);
    else
        infoRadioButton[5]->setEnabled(false);
    infoRadioButton[5]->show();


    infoRadioButton[6]->setGeometry(40,240,240,20);
    infoRadioButton[6]->setText("SBM100 setting");
    if(isSBM100Exist)
        infoRadioButton[6]->setEnabled(true);
    else
        infoRadioButton[6]->setEnabled(false);
    infoRadioButton[6]->show();

    //infoRadioButton[4]->setGeometry(40,180,240,20);
    //infoRadioButton[4]->setText("Show Reference Frequency Response");
    //infoRadioButton[4]->setEnabled(false);
    //infoRadioButton[4]->show();

    //infoRadioButton[5]->setGeometry(40,210,240,20);
    //infoRadioButton[5]->setText("Edit Reference Frequency Response");
    //infoRadioButton[5]->setEnabled(false);
    //infoRadioButton[5]->show();


    for(int i=0;i<7;i++)
    {
        infoBtnGrup->addButton(infoRadioButton[i]);
        infoBtnGrup->setId(infoRadioButton[i],i);
    }


    infoBtnGrup->button(0)->setChecked(true);


    connect(infoBtnGrup,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(infoRadiobtnGroup1clicked(QAbstractButton*)));

}

void MainWindow::initPage0()
{
    qDebug() << "[MainWindow]initPage0";
    for(int i=0;i<PAGE0_REG_MAX_SIZE;i++)
    {
        ui->tableWidget_page0->item(i,0)->setText(page0Manager.getAddress(i));
        ui->tableWidget_page0->item(i,1)->setText(page0Manager.getAcronym(i));
        ui->tableWidget_page0->item(i,2)->setText(page0Manager.getValue(i));
    }

    qDebug() << "[MainWindow]initPage0";
}

void MainWindow::initPage2()
{
    qDebug() << "[MainWindow]initPage2";
    for(int i=0;i<PAGE2_REG_MAX_SIZE;i++)
    {
        ui->tableWidget_page2->item(i,0)->setText(page2Manager.getAddress(i));
        ui->tableWidget_page2->item(i,1)->setText(page2Manager.getAcronym(i));
        ui->tableWidget_page2->item(i,2)->setText(page2Manager.getValue(i));
    }
}

void MainWindow::initPage3()
{
    qDebug() << "[MainWindow]initPage3";
    for(int i=0;i<PAGE3_REG_MAX_SIZE;i++)
    {
        ui->tableWidget_page3->item(i,0)->setText(page3Manager.getAddress(i));
        ui->tableWidget_page3->item(i,1)->setText(page3Manager.getAcronym(i));
        ui->tableWidget_page3->item(i,2)->setText(page3Manager.getValue(i));
    }
}

void MainWindow::initPage4()
{
    qDebug() << "[MainWindow]initPage4";
    for(int i=0;i<PAGE4_REG_MAX_SIZE;i++)
    {
        ui->tableWidget_page4->item(i,0)->setText(page4Manager.getAddress(i));
        ui->tableWidget_page4->item(i,1)->setText(page4Manager.getAcronym(i));
        ui->tableWidget_page4->item(i,2)->setText(page4Manager.getValue(i));
    }
}

bool MainWindow::checkMCP2221SerialNumberEnumerationEnable(unsigned int VID, unsigned int PID)
{
    QString error_string;
    bool bReconnect=false;

    qDebug() << "[MainWindow]checkMCP2221SerialNumberEnumerationEnable";

    //Get number of connected devices with this VID & PID
    Mcp2221_GetConnectedDevices(VID, PID, &NumOfDev);
    if(NumOfDev == 0)
    {
        error_string = "No MCP2221 devices connected";
    }
    else
    {
        error_string = "Number of devices found: " + QString::number(NumOfDev);
    }
    qDebug() << "[MainWindow]"<<error_string;

    for(unsigned int index=0;index<NumOfDev;index++)
    {
        unsigned char snEnumEnabled;
        handle = Mcp2221_OpenByIndex(VID, PID, index);

        if(handle==NULL)
            continue;
        //Get Factory serial number descriptor
        flag = Mcp2221_GetFactorySerialNumber(handle, PordSerNum);
        if(flag == 0)
        {
            error_string = "Factory serial number: " + QString::fromWCharArray(PordSerNum);
        }
        else
        {
            error_string = "Error getting serial number:" + QString::number(flag);
        }

        qDebug() << "[MainWindow]"<<error_string;

        Mcp2221_GetSerialNumberEnumerationEnable(handle, &snEnumEnabled);

        if(snEnumEnabled==0)
        {
            Mcp2221_SetSerialNumberEnumerationEnable(handle, 1);
            Mcp2221_Reset(handle);
            bReconnect=true;
        }
        Mcp2221_Close(handle);
        handle= NULL;
    }

    if(bReconnect)
    {
        QMessageBox::information(NULL, "MessageBox", "Set SerialNumberEnumerationEnable to 1\r\n please reconnect your COM port!!");
        return false;
    }

    return true;
}

bool MainWindow::initMCP2221(QString SerialNum,unsigned int VID, unsigned int PID)
{
    QString error_string;
    bool bMatchSerial=false;

    qDebug() << "[MainWindow]initMCP2221() "<<SerialNum;

    ver = Mcp2221_GetLibraryVersion(LibVer);		//Get DLL version
    if(ver == 0)
    {
        error_string = "Library (DLL) version: "+QString::fromWCharArray(LibVer);
        qDebug() << "[MainWindow]"<<error_string;

    }
    else
    {
        error = Mcp2221_GetLastError();
        error_string = "Cannot get version, error: " + QString::fromStdString(Mcp2221_GetErrorName(error));
        qDebug() << "[MainWindow]"<<error_string;
        return false;
    }

    Mcp2221_GetConnectedDevices(VID, PID, &NumOfDev);
    if(NumOfDev == 0)
    {
        error_string = "No MCP2221 devices connected";
        qDebug() << "[MainWindow]"<<error_string;

         return false;
    }
    else
    {
        error_string = "Number of devices found: " + QString::number(NumOfDev);
        qDebug() << "[MainWindow]"<<error_string;
    }


    for(unsigned int index=0;index<NumOfDev;index++)
    {

        handle = Mcp2221_OpenByIndex(VID, PID, index);

        if(handle==NULL)
            continue;

        //Get product serial number descriptor
        flag = Mcp2221_GetSerialNumberDescriptor(handle, PordSerNum);
        if(flag == 0)
        {
            error_string = "Product serial number: " + QString::fromWCharArray(PordSerNum);
        }
        else
        {
            error_string = "Error getting serial number:" + QString::number(flag);
        }

        qDebug() << "[MainWindow]"<<error_string;

        QString tempSerial=QString::fromWCharArray(PordSerNum);

        if(tempSerial==SerialNum)
        {
            bMatchSerial=true;
            break;
        }
        else
        {

            Mcp2221_Close(handle);
            handle= NULL;
        }
    }

    if(!bMatchSerial)
        return false;

    flag = Mcp2221_GetVidPid(handle, &mcpVID, &mcpPID);
    flag = Mcp2221_GetManufacturerDescriptor(handle, MfrDescriptor);
    if(flag == 0)
    {
        error_string = "VID:0x" + QString("%1").arg(mcpVID, 4, 16, QLatin1Char('0')).toUpper()+", PID:0x"+QString("%1").arg(mcpPID, 4, 16, QLatin1Char('0')).toUpper();
    }
    else
    {
        error_string = "Error getting vidpid: " + QString::number(flag);
    }

    qDebug() << "[MainWindow]"<<error_string;


    //Get manufacturer descriptor
    flag = Mcp2221_GetManufacturerDescriptor(handle, MfrDescriptor);
    if(flag == 0)
    {
        error_string = "Manufacturer descriptor: " + QString::fromWCharArray(MfrDescriptor);
    }
    else
    {
        error_string = "Error getting descriptor: " + QString::number(flag);
    }

    qDebug() << "[MainWindow]"<<error_string;

    //Get product descriptor
    flag = Mcp2221_GetProductDescriptor(handle, ProdDescrip);
    if(flag == 0)
    {
        error_string = "Product descriptor: " + QString::fromWCharArray(ProdDescrip);
    }
    else
    {
        error_string = "Error getting product descriptor:" + QString::number(flag);
    }

    qDebug() << "[MainWindow]"<<error_string;

    //Get product serial number descriptor
    flag = Mcp2221_GetSerialNumberDescriptor(handle, PordSerNum);
    if(flag == 0)
    {
        error_string = "Product serial number: " + QString::fromWCharArray(PordSerNum);
    }
    else
    {
        error_string = "Error getting serial number:" + QString::number(flag);
    }

    qDebug() << "[MainWindow]"<<error_string;

    //Get power attributes
    flag = Mcp2221_GetUsbPowerAttributes(handle, &PowerAttrib, &ReqCurrent);
    if(flag == 0)
    {
        error_string = "Power Attributes " + QString::number(PowerAttrib) + "\nRequested current units = " + QString::number(ReqCurrent) + "\nRequested current(mA) = " + QString::number(ReqCurrent*2);
    }
    else
    {
        error_string = "Error getting power attributes:"+ QString::number(flag);
    }

    qDebug() << "[MainWindow]"<<error_string;

    if(ui->tabWidget->currentIndex()==4)
    {
        onTabChanged(4);
    }
    //init gpio function
    unsigned char pinFunc[4] = {MCP2221_GPFUNC_IO, MCP2221_GPFUNC_IO, MCP2221_GPFUNC_IO, MCP2221_GPFUNC_IO};
    unsigned char pinDir[4] = {MCP2221_GPDIR_OUTPUT, MCP2221_GPDIR_OUTPUT, MCP2221_GPDIR_OUTPUT, MCP2221_GPDIR_INPUT};
    unsigned char OutValues[4] = {MCP2221_GPVAL_LOW, MCP2221_GPVAL_LOW, MCP2221_GPVAL_HIGH, NO_CHANGE};
    unsigned char tempPinFunc[4],tempPinDir[4],tempOutValues[4];
    bool bWriteFlashSettings=false;

    Mcp2221_GetGpioSettings(handle,FLASH_SETTINGS,tempPinFunc,tempPinDir,tempOutValues);

    for(int i=0;i<4;i++)
    {
        if((pinFunc[i]!=tempPinFunc[i])||(pinDir[i]!=tempPinDir[i])){
            bWriteFlashSettings=true;
            break;
        }

        if(i<3){
            if(OutValues[i]!=tempOutValues[i])
            {
                bWriteFlashSettings=true;
                break;
            }
        }
    }

    if(bWriteFlashSettings){
        qDebug() << "FLASH_SETTINGS";
        Mcp2221_SetGpioSettings(handle, FLASH_SETTINGS, pinFunc, pinDir, OutValues);
    }else{
        qDebug() << "RUNTIME_SETTINGS";
        Mcp2221_SetGpioSettings(handle, RUNTIME_SETTINGS, pinFunc, pinDir, OutValues);
    }

    updateUartChannel(ui->uartCH_comboBox->currentIndex());

    return true;
}


void MainWindow::enableFWLog(uint8_t bEnable)
{
    qDebug() << "[MainWindow]enableFWLog()";
    uint8_t data[1];
    data[0]=bEnable;
    GenCommand(0xf0,data,1);
}

void MainWindow::enableWatchdog(uint8_t bEnable)
{
    qDebug() << "[MainWindow]enableWatchdog()";
    uint8_t data[1];
    data[0]=bEnable;
    GenCommand(0xf1,data,1);
}

void MainWindow::refreshNote()
{
    qDebug() << "[MainWindow]refreshNote()";
    uint8_t page[1];
    page[0]=BLOCK_ID_INFO2;
    GenCommand(READ_EEPROM_BLOCK_CMD,page,1);
}

void MainWindow::refreshInfo()
{
    qDebug() << "[MainWindow]refreshInfo()";
    uint8_t page[1];
    page[0]=BLOCK_ID_INFO1;
    GenCommand(READ_EEPROM_BLOCK_CMD,page,1);
}

void MainWindow::updateUartChannel(int index)
{
    qDebug() << "[MainWindow]updateUartChannel() index:"<<index;

    unsigned char OutValues[4] = {MCP2221_GPVAL_LOW, MCP2221_GPVAL_LOW, NO_CHANGE, NO_CHANGE};
    qDebug() << index;

    if(handle==NULL)
    {
        qDebug() << "[MainWindow]updateUartChannel() handle null";
        return;
    }

    if(index==0)
    {
        OutValues[0]=MCP2221_GPVAL_LOW;
        OutValues[1]=MCP2221_GPVAL_LOW;
        OutValues[2]=NO_CHANGE;
        OutValues[3]=NO_CHANGE;
    }
    else if(index==1)
    {
        OutValues[0]=MCP2221_GPVAL_HIGH;
        OutValues[1]=MCP2221_GPVAL_LOW;
        OutValues[2]=NO_CHANGE;
        OutValues[3]=NO_CHANGE;
    }
    else if(index==2)
    {
        OutValues[0]=MCP2221_GPVAL_LOW;
        OutValues[1]=MCP2221_GPVAL_HIGH;
        OutValues[2]=NO_CHANGE;
        OutValues[3]=NO_CHANGE;
    }
    else if(index==3)
    {
        OutValues[0]=MCP2221_GPVAL_HIGH;
        OutValues[1]=MCP2221_GPVAL_HIGH;
        OutValues[2]=NO_CHANGE;
        OutValues[3]=NO_CHANGE;
    }

    Mcp2221_SetGpioValues(handle, OutValues);

    syncDeviceData();
}

void MainWindow::syncDeviceData()
{
    if(!IsConnect)
        return;

    checkDSP368RetryCount=0;

    GenCommand(GET_FW_VERSION_CMD, NULL, 0);

    GenCommand(READ_DPS368_COEFF_CMD, NULL, 0);
    sbm100_readFrom_clicked();
    if(enableCal)
        GenCommand(READ_SHT4X_CAL_CMD,NULL,0);
    GenCommand(GET_SHT4X_SERIAL_NUM_CMD, NULL, 0);

    refreshInfo();
    refreshNote();

    if(!timer->isActive())
    {
        if(sht4xEnable)
        {
            readOutSHT4x();
            readOutIntervalCount=0;
            bWaitReadOutResponse=false;
            waitReadOutResponseCount=0;
        }

        if(dsp368Enable)
        {
            readOutDSP368();
            dsp368ReadOutIntervalCount=0;
            bWaitDsp368ReadOutResponse=false;
            waitDsp368ReadOutResponseCount=0;
        }
        timer->start();
    }

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    FW_Debug_Msg  = settings.value("FW_Debug_Msg/Enable", "0").toInt();
    if(FW_Debug_Msg>1)
        FW_Debug_Msg=0;

    enableFWLog(FW_Debug_Msg);

    if(needAutoRefreshWhenConnect==1)
    {
        page0RefreshBtn_clicked();
        page2RefreshBtn_clicked();
        page3RefreshBtn_clicked();
        page4RefreshBtn_clicked();
    }

    WATCHDOG = settings.value("WATCHDOG/Enable", "0").toInt();

    if(WATCHDOG>1)
        WATCHDOG=0;

    enableWatchdog(WATCHDOG);
}

void MainWindow::uartChannelChange(int index)
{
    updateUartChannel(index);
}

void MainWindow::comPortConnect()
{
    qDebug() << "[MainWindow]comPortConnect()";
    IsConnect=true;
    bCheckDeviceOffLine=false;
    bReSyncDataWhenDeviceOnLine=false;
    ui->comPort_comboBox->setEnabled(false);
    ui->disconnectBtn->setEnabled(true);
    ui->upgradeBtn->setEnabled(true);
    ui->connectBtn->setEnabled(false);
    mSerial->clear(QSerialPort::AllDirections);
    mSerial->readAll();
    mSerial->flush();

    syncDeviceData();
}

void MainWindow::comPortDisconnect()
{
    qDebug() << "[MainWindow]comPortDisconnect()";

    IsConnect=false;

    if(timer->isActive())
        timer->stop();

    readOutIntervalCount=0;
    waitReadOutResponseCount=0;
    bWaitReadOutResponse=false;
    dsp368ReadOutIntervalCount=0;
    waitDsp368ReadOutResponseCount=0;
    bWaitDsp368ReadOutResponse=false;

    reConnectSyncCount=0;
    SendCommandInterval=0;
    waitCMDResponseCount=0;
    bCheckCMDSend=false;
    reConnectSync=false;
    bDisconnect=false;

    Mcp2221_CloseAll();
    handle = NULL;
    if (mSerial->isOpen()) {
        qDebug() << "Serial already connected, disconnecting!";
        mSerial->close();
    }
    //ui->temperatureLabel->setText("Temperature:");
    //ui->humidtyLabel->setText("Humidty:");
    m_sht4xCard->clearValue1();
    m_sht4xCard->clearValue2();
    //ui->pressureLabel->setText("Pressure:");
    //ui->tempDPS368Label->setText("Temperature:");
    m_dsp368Card->clearValue1();
    m_dsp368Card->clearValue2();
    //ui->deviceIDlabel->setText("");
    //setWindowTitle(AppVersion+" Device ID:");
    m_deviceidCard->clearDeviceId();
    ui->comPort_comboBox->setEnabled(true);
    ui->connectBtn->setEnabled(true);
    ui->disconnectBtn->setEnabled(false);
    ui->upgradeBtn->setEnabled(false);
    ui->progressBar->setValue(0);

}

void MainWindow::upgradeBtn_clicked()
{
    qDebug() << "[MainWindow]upgradeBtn_clicked()";
    if(!IsConnect)
        return;

    if(devID!=DEV_ID_NUMBER)
    {
        QMessageBox::information(NULL, "MessageBox", "not eMic device!!");
        return;
    }

    if(imageVersion.contains("file error"))
    {
        QMessageBox::information(NULL, "MessageBox", "file error!!");
        return;
    }

    if(!file.exists())
    {
        QMessageBox::information(NULL, "MessageBox", "Please select File");
        return;
    }

    bUpgrade=true;
    ui->upgradeBtn->setEnabled(false);
    ui->tabWidget->setTabEnabled(0,false);
    ui->tabWidget->setTabEnabled(1,false);
    ui->tabWidget->setTabEnabled(2,false);
    ui->tabWidget->setTabEnabled(3,false);
    ui->tabWidget->setTabEnabled(4,false);

    UpgradePos = 0x0;
    ImageCheckSum=0;

    range = (short)(FileSize / (FileSize/100));
    qDebug() <<"filesize:" <<FileSize;

    ui->progressBar->setRange(0,range);
    ui->progressBar->setValue(0);

    file.seek(0x2000);

    mSerial->flush();

    GenCommand(0xb0, NULL,0);
}

void MainWindow::page0WriteRegBtn_clicked()
{
    qDebug() << "[MainWindow]page0WriteRegBtn_clicked()";
    uint8_t page[3];
    if(ui->tableWidget_page0->currentRow()<0)
    {
        qDebug() << "[MainWindow]Not select Reg";
        QMessageBox::information(NULL, "MessageBox", "Please select REG");
    }
    else
    {
        page[0]=0;
        page[1]=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),0)->text()).toInt(NULL,16);
        page[2]=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
        qDebug() << "page[1]:"<<page[1];
        qDebug() << "page[2]:"<<page[2];
        GenCommand(WRITE_PCMD_REG_WITH_PAGE_PARAMETERS_CMD,page,3);
    }
}

void MainWindow::page2WriteRegBtn_clicked()
{
    uint8_t page[3];

    qDebug() << "[MainWindow]page2WriteRegBtn_clicked()";
    if(ui->tableWidget_page2->currentRow()<0)
    {
        qDebug() << "[MainWindow]Not select Reg";
        QMessageBox::information(NULL, "MessageBox", "Please select REG");
    }
    else
    {
        page[0]=2;
        page[1]=(ui->tableWidget_page2->item(ui->tableWidget_page2->currentRow(),0)->text()).toInt(NULL,16);
        page[2]=(ui->tableWidget_page2->item(ui->tableWidget_page2->currentRow(),2)->text()).toInt(NULL,16);
        qDebug() << "page[1]:"<<page[1];
        qDebug() << "page[2]:"<<page[2];
        GenCommand(WRITE_PCMD_REG_WITH_PAGE_PARAMETERS_CMD,page,3);
    }
}

void MainWindow::page3WriteRegBtn_clicked()
{
    uint8_t page[3];

    qDebug() << "[MainWindow]page3WriteRegBtn_clicked()";
    if(ui->tableWidget_page3->currentRow()<0)
    {
        qDebug() << "[MainWindow]Not select Reg";
        QMessageBox::information(NULL, "MessageBox", "Please select REG");
    }
    else
    {
        page[0]=3;
        page[1]=(ui->tableWidget_page3->item(ui->tableWidget_page3->currentRow(),0)->text()).toInt(NULL,16);
        page[2]=(ui->tableWidget_page3->item(ui->tableWidget_page3->currentRow(),2)->text()).toInt(NULL,16);
        qDebug() << "page[1]:"<<page[1];
        qDebug() << "page[2]:"<<page[2];
        GenCommand(WRITE_PCMD_REG_WITH_PAGE_PARAMETERS_CMD,page,3);
    }
}

void MainWindow::page4WriteRegBtn_clicked()
{
    uint8_t page[3];

    qDebug() << "[MainWindow]page4WriteRegBtn_clicked()";
    if(ui->tableWidget_page4->currentRow()<0)
    {
        qDebug() << "[MainWindow]Not select Reg";
        QMessageBox::information(NULL, "MessageBox", "Please select REG");
    }
    else
    {
        page[0]=4;
        page[1]=(ui->tableWidget_page4->item(ui->tableWidget_page4->currentRow(),0)->text()).toInt(NULL,16);
        page[2]=(ui->tableWidget_page4->item(ui->tableWidget_page4->currentRow(),2)->text()).toInt(NULL,16);
        qDebug() << "page[1]:"<<page[1];
        qDebug() << "page[2]:"<<page[2];
        GenCommand(WRITE_PCMD_REG_WITH_PAGE_PARAMETERS_CMD,page,3);
    }
}

void MainWindow::page0WriteAllBtn_clicked()
{
    uint8_t pageBuf[PAGE0_REG_MAX_SIZE+2];

    qDebug() << "[MainWindow]page0WriteAllBtn_clicked()";

    pageBuf[0]=0;

    for(int i=1;i<PAGE0_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page0->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE0_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE0_REG_MAX_SIZE);
    GenCommand(WRITE_PCMD_BLOCK_CMD, pageBuf,PAGE0_REG_MAX_SIZE+2);
}

void MainWindow::page2WriteAllBtn_clicked()
{
    uint8_t pageBuf[PAGE2_REG_MAX_SIZE+2];

    qDebug() << "[MainWindow]page2WriteAllBtn_clicked()";

    pageBuf[0]=2;

    for(int i=1;i<PAGE2_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page2->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE2_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE2_REG_MAX_SIZE);
    GenCommand(WRITE_PCMD_BLOCK_CMD, pageBuf,PAGE2_REG_MAX_SIZE+2);
}

void MainWindow::page3WriteAllBtn_clicked()
{
    uint8_t pageBuf[PAGE3_REG_MAX_SIZE+2];

    qDebug() << "[MainWindow]page3WriteAllBtn_clicked()";

    pageBuf[0]=3;

    for(int i=1;i<PAGE3_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page3->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE3_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE3_REG_MAX_SIZE);
    GenCommand(WRITE_PCMD_BLOCK_CMD, pageBuf,PAGE3_REG_MAX_SIZE+2);
}

void MainWindow::page4WriteAllBtn_clicked()
{
    uint8_t pageBuf[PAGE4_REG_MAX_SIZE+2];

    qDebug() << "[MainWindow]page4WriteAllBtn_clicked()";

    pageBuf[0]=4;

    for(int i=1;i<PAGE4_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page4->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE4_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE4_REG_MAX_SIZE);
    GenCommand(WRITE_PCMD_BLOCK_CMD, pageBuf,PAGE4_REG_MAX_SIZE+2);
}

QString MainWindow::normalizeHexByte(QString str) {
    if (str.startsWith("0x") && str.length() == 4) {
        return "0x" + str.mid(2).toUpper();
    }
    return str;  // If the format does not match, it will be returned as is
}

bool MainWindow::isValidHexByte(const QString &str)
{
    static QRegularExpression re("^0x[0-9A-Fa-f]{2}$");
    return re.match(str).hasMatch();
}

bool MainWindow::checkImportCSVFormat(QTableWidget *tableWidget, const QString &filePath)
{
    qDebug() << "[MainWindow]checkImportCSVFormat()";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);

    int row = 0;
    bool result=true;
    QString headerLine = in.readLine(); // Skip header

    Q_UNUSED(headerLine);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() < 3) continue;

        qDebug() << tableWidget->item(row,0)->text();
        qDebug() << "=====================================";
        qDebug() << tableWidget->item(row,1)->text();
        qDebug() << "=====================================";

        if(tableWidget->item(row,0)->text()!=parts[0].trimmed()||tableWidget->item(row,1)->text()!=parts[1].trimmed())
        {
            qDebug() <<"[MainWindow]Format error in row:"<<row;
            qDebug() <<"[MainWindow]"<< tableWidget->item(row,0)->text()<<","<<parts[0].trimmed();
            qDebug() <<"[MainWindow]"<< tableWidget->item(row,1)->text()<<","<<parts[1].trimmed();
            result = false;
            break;
        }
        else
        {
            if(isValidHexByte(parts[2].trimmed())==false)
            {
                qDebug() << "[MainWindow]Format error Value not valid hex byte:"<<"row:"<<parts[2].trimmed();
                result = false;
                break;
            }
        }

        ++row;
    }

    file.close();

    return result;
}

void MainWindow::importFromCSV(QTableWidget *tableWidget, const QString &filePath) {

    qDebug() << "[MainWindow]importFromCSV()";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "[MainWindow]csv file open fail";
        return;
    }

    QTextStream in(&file);

    int row = 0;
    QString headerLine = in.readLine(); // Skip header

    Q_UNUSED(headerLine);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() < 3) continue;

        if(parts[1].trimmed()!="PAGE_CFG"&&parts[1].trimmed()!="I2C_CKSUM")
        {
            tableWidget->item(row,2)->setText(normalizeHexByte(parts[2].trimmed()));
        }

        ++row;
    }

    file.close();
}


void MainWindow::exportToCSV(QTableWidget *tableWidget, const QString &filePath) {

    QFile file(filePath);

    qDebug() << "[MainWindow]exportToCSV()";

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ))
    {
        qDebug() << "[MainWindow]csv file open fail";
        return;
    }

    QTextStream out(&file);
    out << "Address,Acronym,Value(hex)\n";

    int rows = tableWidget->rowCount();
    for (int i = 0; i < rows; ++i) {
        QString addr = tableWidget->item(i, 0) ? tableWidget->item(i, 0)->text() : "";
        QString name = tableWidget->item(i, 1) ? tableWidget->item(i, 1)->text() : "";
        QString value = tableWidget->item(i, 2) ? tableWidget->item(i, 2)->text() : "";

        out << addr << "," << name << "," << value << "\n";
    }

    file.close();
}

void MainWindow::autoImportCSVFile()
{
    qDebug() << "[MainWindow]autoImportCSVFile()";

    QString filePath = QCoreApplication::applicationDirPath()+"/default_config_csv/page0.csv";

    if (!filePath.isEmpty()) {

        if(checkImportCSVFormat(ui->tableWidget_page0, filePath))
        {
            importFromCSV(ui->tableWidget_page0, filePath);
            if(ui->tabWidget->currentIndex()==0)
            {
                onTabChanged(0);
            }
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "page0 CSV file format error!!");
        }
    }

    filePath = QCoreApplication::applicationDirPath()+"/default_config_csv/page2.csv";

    if (!filePath.isEmpty()) {

        if(checkImportCSVFormat(ui->tableWidget_page2, filePath))
        {
            importFromCSV(ui->tableWidget_page2, filePath);
            if(ui->tabWidget->currentIndex()==1)
            {
                onTabChanged(1);
            }
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "page2 CSV file format error!!");
        }
    }

    filePath = QCoreApplication::applicationDirPath()+"/default_config_csv/page3.csv";
    if (!filePath.isEmpty()) {

        if(checkImportCSVFormat(ui->tableWidget_page3, filePath))
        {
            importFromCSV(ui->tableWidget_page3, filePath);
            if(ui->tabWidget->currentIndex()==2)
            {
                onTabChanged(2);
            }
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "page3 CSV file format error!!");
        }
    }

    filePath = QCoreApplication::applicationDirPath()+"/default_config_csv/page4.csv";

    if (!filePath.isEmpty()) {

        if(checkImportCSVFormat(ui->tableWidget_page4, filePath))
        {
            importFromCSV(ui->tableWidget_page4, filePath);
            if(ui->tabWidget->currentIndex()==3)
            {
                onTabChanged(3);
            }
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "page4 CSV file format error!!");
        }
    }


}

void MainWindow::page0ExportBtn_clicked()
{
    qDebug() << "[MainWindow]page0ExportBtn_clicked()";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    QString lastPath = settings.value("CSV_File/Export_Last_Open_Dir", QDir::currentPath()).toString();
    QString filePath = QFileDialog::getSaveFileName(this, "Export CSV", lastPath, "CSV files (*.csv)");
    if (!filePath.isEmpty()) {

        QFileInfo fi(filePath);
        settings.setValue("CSV_File/Export_Last_Open_Dir", fi.absolutePath());

        exportToCSV(ui->tableWidget_page0, filePath);
    }
}

void MainWindow::page0ImportBtn_clicked()
{
    qDebug() << "[MainWindow]page0ImportBtn_clicked()";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    QString lastPath = settings.value("CSV_File/Import_Last_Open_Dir", QDir::currentPath()).toString();
    QString filePath = QFileDialog::getOpenFileName(this, "Import CSV", lastPath, "CSV files (*.csv)");
    if (!filePath.isEmpty()) {

        QFileInfo fi(filePath);
        settings.setValue("CSV_File/Import_Last_Open_Dir", fi.absolutePath());
        settings.setValue("SAVE_ALL/PAGE0_CSV_FILE",filePath);

        if(checkImportCSVFormat(ui->tableWidget_page0, filePath))
        {
            importFromCSV(ui->tableWidget_page0, filePath);
            onTabChanged(0);
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "CSV file format error!!");
        }
    }
}


void MainWindow::page2ExportBtn_clicked()
{
    qDebug() << "[MainWindow]page2ExportBtn_clicked()";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    QString lastPath = settings.value("CSV_File/Export_Last_Open_Dir", QDir::currentPath()).toString();
    QString filePath = QFileDialog::getSaveFileName(this, "Export CSV", lastPath, "CSV files (*.csv)");
    if (!filePath.isEmpty()) {

        QFileInfo fi(filePath);
        settings.setValue("CSV_File/Export_Last_Open_Dir", fi.absolutePath());

        exportToCSV(ui->tableWidget_page2, filePath);
    }
}

void MainWindow::page2ImportBtn_clicked()
{
    qDebug() << "[MainWindow]page2ImportBtn_clicked()";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    QString lastPath = settings.value("CSV_File/Import_Last_Open_Dir", QDir::currentPath()).toString();
    QString filePath = QFileDialog::getOpenFileName(this, "Import CSV", lastPath, "CSV files (*.csv)");
    if (!filePath.isEmpty()) {

        QFileInfo fi(filePath);
        settings.setValue("CSV_File/Import_Last_Open_Dir", fi.absolutePath());
        settings.setValue("SAVE_ALL/PAGE2_CSV_FILE",filePath);

        if(checkImportCSVFormat(ui->tableWidget_page2, filePath))
        {
            importFromCSV(ui->tableWidget_page2, filePath);
            onTabChanged(1);
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "CSV file format error!!");
        }
    }
}

void MainWindow::page3ExportBtn_clicked()
{
    qDebug() << "[MainWindow]page3ExportBtn_clicked()";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    QString lastPath = settings.value("CSV_File/Export_Last_Open_Dir", QDir::currentPath()).toString();
    QString filePath = QFileDialog::getSaveFileName(this, "Export CSV", lastPath, "CSV files (*.csv)");
    if (!filePath.isEmpty()) {

        QFileInfo fi(filePath);
        settings.setValue("CSV_File/Export_Last_Open_Dir", fi.absolutePath());

        exportToCSV(ui->tableWidget_page3, filePath);
    }
}

void MainWindow::page3ImportBtn_clicked()
{
    qDebug() << "[MainWindow]page3ImportBtn_clicked()";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    QString lastPath = settings.value("CSV_File/Import_Last_Open_Dir", QDir::currentPath()).toString();
    QString filePath = QFileDialog::getOpenFileName(this, "Import CSV", lastPath, "CSV files (*.csv)");
    if (!filePath.isEmpty()) {

        QFileInfo fi(filePath);
        settings.setValue("CSV_File/Import_Last_Open_Dir", fi.absolutePath());
        settings.setValue("SAVE_ALL/PAGE3_CSV_FILE",filePath);

        if(checkImportCSVFormat(ui->tableWidget_page3, filePath))
        {
            importFromCSV(ui->tableWidget_page3, filePath);
            onTabChanged(2);
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "CSV file format error!!");
        }
    }
}

void MainWindow::page4ExportBtn_clicked()
{
    qDebug() << "[MainWindow]page4ExportBtn_clicked()";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    QString lastPath = settings.value("CSV_File/Export_Last_Open_Dir", QDir::currentPath()).toString();
    QString filePath = QFileDialog::getSaveFileName(this, "Export CSV", lastPath, "CSV files (*.csv)");
    if (!filePath.isEmpty()) {

        QFileInfo fi(filePath);
        settings.setValue("CSV_File/Export_Last_Open_Dir", fi.absolutePath());

        exportToCSV(ui->tableWidget_page4, filePath);
    }
}

void MainWindow::page4ImportBtn_clicked()
{
    qDebug() << "[MainWindow]page4ImportBtn_clicked()";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    QString lastPath = settings.value("CSV_File/Import_Last_Open_Dir", QDir::currentPath()).toString();
    QString filePath = QFileDialog::getOpenFileName(this, "Import CSV", lastPath, "CSV files (*.csv)");
    if (!filePath.isEmpty()) {

        QFileInfo fi(filePath);
        settings.setValue("CSV_File/Import_Last_Open_Dir", fi.absolutePath());
        settings.setValue("SAVE_ALL/PAGE4_CSV_FILE",filePath);

        if(checkImportCSVFormat(ui->tableWidget_page4, filePath))
        {
            importFromCSV(ui->tableWidget_page4, filePath);
            onTabChanged(3);
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "CSV file format error!!");
        }
    }
}

void  MainWindow::refreshPage(uint8_t pageID,uint8_t readFrom)
{
    qDebug() << "[MainWindow]refreshPage() pageID:"<<pageID<<" readFrom"<<readFrom;

    uint8_t page[1];
    if(readFrom==0)
    {
        page[0]=pageID;
        GenCommand(READ_EEPROM_BLOCK_CMD,page,1);
    }
    else
    {
        if(pageID==BLOCK_ID_PAGE0)
            page[0]=0;
        else if(pageID==BLOCK_ID_PAGE2)
            page[0]=2;
        else if(pageID==BLOCK_ID_PAGE3)
            page[0]=3;
        else if(pageID==BLOCK_ID_PAGE4)
            page[0]=4;

        GenCommand(READ_PCMD_BLOCK_CMD,page,1);
    }
}

void MainWindow::page0RefreshBtn_clicked()
{
    uint8_t readFrom=0;

    qDebug() << "[MainWindow]page0RefreshBtn_clicked()";

    readFrom =  ui->page0ReadMethod_comboBox->currentIndex();

    refreshPage(BLOCK_ID_PAGE0,readFrom);
}

void MainWindow::page2RefreshBtn_clicked()
{
    uint8_t readFrom=0;

    qDebug() << "[MainWindow]page2RefreshBtn_clicked()";

    readFrom =  ui->page2ReadMethod_comboBox->currentIndex();

    refreshPage(BLOCK_ID_PAGE2,readFrom);
}

void MainWindow::page3RefreshBtn_clicked()
{
    uint8_t readFrom=0;

    qDebug() << "[MainWindow]page3RefreshBtn_clicked()";

    readFrom =  ui->page3ReadMethod_comboBox->currentIndex();

    refreshPage(BLOCK_ID_PAGE3,readFrom);
}

void MainWindow::page4RefreshBtn_clicked()
{
    uint8_t readFrom=0;

    qDebug() << "[MainWindow]page4RefreshBtn_clicked()";

    readFrom =  ui->page4ReadMethod_comboBox->currentIndex();

    refreshPage(BLOCK_ID_PAGE4,readFrom);
}

void MainWindow::savePage0()
{

    uint8_t pageBuf[PAGE0_REG_MAX_SIZE+2];

    pageBuf[0]=BLOCK_ID_PAGE0;

    for(int i=1;i<PAGE0_REG_MAX_SIZE+1;i++)
    {
        if(page0Manager.getProperty(i-1)==REG_READ_ONLY)
        {
            pageBuf[i]=page0Manager.getValueAsByte(i-1);
        }
        else
        {
            pageBuf[i]=(ui->tableWidget_page0->item(i-1,2)->text()).toInt(NULL,16);
        }
    }

    pageBuf[PAGE0_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE0_REG_MAX_SIZE);
    GenCommand(WRITE_EEPROM_BLOCK_CMD, pageBuf,PAGE0_REG_MAX_SIZE+2);
}

void MainWindow::savePage2()
{

    uint8_t pageBuf[PAGE2_REG_MAX_SIZE+2];



    pageBuf[0]=BLOCK_ID_PAGE2;

    for(int i=1;i<PAGE2_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page2->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE2_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE2_REG_MAX_SIZE);
    GenCommand(WRITE_EEPROM_BLOCK_CMD, pageBuf,PAGE2_REG_MAX_SIZE+2);
}

void MainWindow::savePage3()
{

    uint8_t pageBuf[PAGE3_REG_MAX_SIZE+2];

    pageBuf[0]=BLOCK_ID_PAGE3;

    for(int i=1;i<PAGE3_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page3->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE3_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE3_REG_MAX_SIZE);
    GenCommand(WRITE_EEPROM_BLOCK_CMD, pageBuf,PAGE3_REG_MAX_SIZE+2);
}

void MainWindow::savePage4()
{

    uint8_t pageBuf[PAGE4_REG_MAX_SIZE+2];

    pageBuf[0]=BLOCK_ID_PAGE4;

    for(int i=1;i<PAGE4_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page4->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE4_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE4_REG_MAX_SIZE);
    GenCommand(WRITE_EEPROM_BLOCK_CMD, pageBuf,PAGE4_REG_MAX_SIZE+2);
}

void MainWindow::importCSVBeforeSave()
{
    qDebug() << "[MainWindow]importCSVBeforeSave()";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    Import_CSV_File_Before_Write = settings.value("SAVE_ALL/Import_CSV_File_Before_Write", "0").toInt();

    if(Import_CSV_File_Before_Write==0)
        return;

    QString filePath = settings.value("SAVE_ALL/PAGE0_CSV_FILE", QDir::currentPath()).toString();

    if (!filePath.isEmpty()) {

        if(checkImportCSVFormat(ui->tableWidget_page0, filePath))
        {
            importFromCSV(ui->tableWidget_page0, filePath);
            if(ui->tabWidget->currentIndex()==0)
            {
                onTabChanged(0);
            }
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "page0 CSV file format error!!");
        }
    }

    filePath = settings.value("SAVE_ALL/PAGE2_CSV_FILE", QDir::currentPath()).toString();

    if (!filePath.isEmpty()) {

        if(checkImportCSVFormat(ui->tableWidget_page2, filePath))
        {
            importFromCSV(ui->tableWidget_page2, filePath);
            if(ui->tabWidget->currentIndex()==1)
            {
                onTabChanged(1);
            }
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "page2 CSV file format error!!");
        }
    }

    filePath = settings.value("SAVE_ALL/PAGE3_CSV_FILE", QDir::currentPath()).toString();
    if (!filePath.isEmpty()) {

        if(checkImportCSVFormat(ui->tableWidget_page3, filePath))
        {
            importFromCSV(ui->tableWidget_page3, filePath);
            if(ui->tabWidget->currentIndex()==2)
            {
                onTabChanged(2);
            }
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "page3 CSV file format error!!");
        }
    }

    filePath = settings.value("SAVE_ALL/PAGE4_CSV_FILE", QDir::currentPath()).toString();

    if (!filePath.isEmpty()) {

        if(checkImportCSVFormat(ui->tableWidget_page4, filePath))
        {
            importFromCSV(ui->tableWidget_page4, filePath);
            if(ui->tabWidget->currentIndex()==3)
            {
                onTabChanged(3);
            }
        }
        else
        {
            QMessageBox::information(NULL, "MessageBox", "page4 CSV file format error!!");
        }
    }
}
void MainWindow::saveAllPages()
{
    importCSVBeforeSave();

    if(writeEEPROMprogress)
    {
        delete writeEEPROMprogress;
        writeEEPROMprogress = nullptr;
    }

    writeEEPROMprogress = new QProgressDialog(this);

    writeEEPROMprogress->setWindowTitle("EEPROM Write");

    writeEEPROMprogress->setLabelText("Start Writing...");

    writeEEPROMprogress->setRange(0, 4);

    writeEEPROMprogress->setCancelButton(nullptr);

    writeEEPROMprogress->setWindowModality(Qt::ApplicationModal);

    writeEEPROMprogress->setWindowFlags(writeEEPROMprogress->windowFlags() & ~Qt::WindowCloseButtonHint);

    writeEEPROMprogress->show();

    writeEEPROMprogress->setLabelText(QString("Writing Page%1...").arg(0));

    writeEEPROMprogress->setValue(0);

    savePage0();
}

void MainWindow::page0SaveAllBtn_clicked()
{
    qDebug() << "[MainWindow]page0SaveAllBtn_clicked()";

    if(bSaveAllFlag||!IsConnect||bDisconnect)
        return;

    bSaveAllFlag=true;

    saveAllPages();
}

void MainWindow::page2SaveAllBtn_clicked()
{
    qDebug() << "[MainWindow]page2SaveAllBtn_clicked()";

    if(bSaveAllFlag||!IsConnect||bDisconnect)
        return;

    bSaveAllFlag=true;

    saveAllPages();
}

void MainWindow::page3SaveAllBtn_clicked()
{
    qDebug() << "[MainWindow]page3SaveAllBtn_clicked()";

    if(bSaveAllFlag||!IsConnect||bDisconnect)
        return;

    bSaveAllFlag=true;

    saveAllPages();
}

void MainWindow::page4SaveAllBtn_clicked()
{
    qDebug() << "[MainWindow]page4SaveAllBtn_clicked()";

    if(bSaveAllFlag||!IsConnect||bDisconnect)
        return;

    bSaveAllFlag=true;

    saveAllPages();
}

void MainWindow::page0SaveBtn_clicked()
{
    qDebug() << "[MainWindow]page0SaveBtn_clicked()";

    bSaveAllFlag=false;
    savePage0();
}

void MainWindow::page2SaveBtn_clicked()
{
    qDebug() << "[MainWindow]page2SaveBtn_clicked()";

    bSaveAllFlag=false;
    savePage2();
}

void MainWindow::page3SaveBtn_clicked()
{
    qDebug() << "[MainWindow]page3SaveBtn_clicked()";

    bSaveAllFlag=false;
    savePage3();
}

void MainWindow::page4SaveBtn_clicked()
{
    qDebug() << "[MainWindow]page4SaveBtn_clicked()";

    bSaveAllFlag=false;
    savePage4();
}



void MainWindow::infoRadiobtnGroup1clicked(QAbstractButton *button)
{
    Q_UNUSED(button);

    qDebug() << "[MainWindow]infoRadiobtnGroup1clicked:"<<infoBtnGrup->checkedId();

    //if(infoBtnGrup->checkedId()==0)
    //{
        //if(bRefreshInfoBlock1)
        //{
        //    refreshInfo();
        //    bRefreshInfoBlock1=false;
        //}

        //if(bRefreshInfoBlock2)
        //{
        //    refreshNote();
        //    bRefreshInfoBlock1=false;
        //}
    //}

    createInfo(infoBtnGrup->checkedId());

}

void MainWindow::DPS368PressureOversampleSlelectChange(int index)
{
    qDebug() << "[MainWindow]DPS368PressureOversampleSlelectChange() index:"<<index;

    dsp368PressureOSRIndex=infoDPS368ComboBox[1]->currentIndex();



    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    settings.setValue("DSP368/PRESSURE_OVERSAMPLE_Type",dsp368PressureOSRIndex);

    //if(dsp368Enable)
    //{
    //    readOutDSP368();
    //    dsp368ReadOutIntervalCount=0;
    //    bWaitDsp368ReadOutResponse=false;
    //    waitDsp368ReadOutResponseCount=0;
    //}

}

void MainWindow::DPS368TempOversampleSlelectChange(int index)
{
    qDebug() << "[MainWindow]DPS368TempOversampleSlelectChange() index:"<<index;

    dsp368TempOSRIndex=infoDPS368ComboBox[0]->currentIndex();



    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    settings.setValue("DSP368/TEMP_OVERSAMPLE_Type",dsp368TempOSRIndex);

    //if(dsp368Enable)
    //{
    //    readOutDSP368();
    //    dsp368ReadOutIntervalCount=0;
    //    bWaitDsp368ReadOutResponse=false;
    //    waitDsp368ReadOutResponseCount=0;
    //}

}

void MainWindow::SMB100_dcb_bypass_SlelectChange(int index)
{
    qDebug() << "[MainWindow]SMB100_dcb_bypass_SlelectChange() index:"<<index;

    dcb_bypass=infoSBM100ComboBox[0]->currentIndex();
}

void MainWindow::SMB100_dcb_cutoff_SlelectChange(int index)
{
    qDebug() << "[MainWindow]SMB100_dcb_cutoff_SlelectChange() index:"<<index;

    dcb_cutoff=infoSBM100ComboBox[1]->currentIndex();
}

void MainWindow::SMB100_biquad_coeff_write_SlelectChange(int index)
{
    qDebug() << "[MainWindow]SMB100_biquad_coeff_write_SlelectChange() index:"<<index;

    biquad_coeff_write=infoSBM100ComboBox[2]->currentIndex();
}

void MainWindow::SMB100_biquad_bypass_SlelectChange(int index)
{
    qDebug() << "[MainWindow]SMB100_biquad_bypass_SlelectChange() index:"<<index;

    biquad_bypass=infoSBM100ComboBox[3]->currentIndex();
}

void MainWindow::SMB100_out24not28_SlelectChange(int index)
{
    qDebug() << "[MainWindow]SMB100_out24not28_SlelectChange() index:"<<index;

    out24not28=infoSBM100ComboBox[4]->currentIndex();
}

void MainWindow::SMB100_polarity_SlelectChange(int index)
{
    qDebug() << "[MainWindow]SMB100_polarity_SlelectChange() index:"<<index;

    polarity=infoSBM100ComboBox[5]->currentIndex();
}

void MainWindow::SMB100_readFrom_SlelectChange(int index)
{
    qDebug() << "[MainWindow]SMB100_readFrom_SlelectChange() index:"<<index;

    readFromIndex=infoSBM100ComboBox[6]->currentIndex();

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    settings.setValue("SMB100/Read_From_Source",readFromIndex);
}

void MainWindow::SMB100_writeTo_SlelectChange(int index)
{
    qDebug() << "[MainWindow]SMB100_writeTo_SlelectChange() index:"<<index;

    writeToIndex=infoSBM100ComboBox[7]->currentIndex();

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    settings.setValue("SMB100/Write_To_Source",writeToIndex);
}

void MainWindow::SMB100_user_gain_index_SlelectChange(int index)
{
    qDebug() << "[MainWindow]SMB100_user_gain_index_SlelectChange:"<<index;

    user_gain_index=index;

    user_gain=user_gain_index&0x1f;

    descriptionLabel[11]->setText("user_gain: "+QString("%1 dB (REG:0x%2)").arg(user_gain_index * 3).arg(user_gain,2,16,QChar('0')));

    qDebug() << "[MainWindow]user_gain:"<<user_gain;

}
int MainWindow::getDPS368ReadOutInterval(int index)
{
    qDebug() << "[MainWindow]getDPS368ReadOutInterval() index:"<<index;
    int interval;
    if(index==0)
    {
        interval=250;
    }
    else if(index==1)
    {
        interval=500;
    }
    else
    {
        interval=(index-1)*1000;
    }

    return interval/TIMER_INTERVAL;

}

void MainWindow::DPS368IntervalSlelectChange(int index)
{
    qDebug() << "[MainWindow]DPS368IntervalSlelectChange() index:"<<index;
    dsp368ReadoutIntervalIndex=infoDPS368ComboBox[2]->currentIndex();

    dsp368ReadoutInterval=getDPS368ReadOutInterval(dsp368ReadoutIntervalIndex);

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    settings.setValue("DSP368/Readout_Interval_Index",dsp368ReadoutIntervalIndex);

    if(dsp368Enable)
    {
        readOutDSP368();
        dsp368ReadOutIntervalCount=0;
        bWaitDsp368ReadOutResponse=false;
        waitDsp368ReadOutResponseCount=0;
    }

}

int MainWindow::getSHT4xReadOutInterval(int index)
{
    qDebug() << "[MainWindow]getDPS368ReadOutInterval() index:"<<index;
    int interval;

    interval=(index+1)*1000;

    return interval/TIMER_INTERVAL;

}

void MainWindow::sht4xIntervalSlelectChange(int index)
{
    qDebug() << "[MainWindow]sht4xCommandSlelectChange() index:"<<index;
    readoutIntervalIndex=infoSHT4xComboBox[1]->currentIndex();

    readoutInterval=getSHT4xReadOutInterval(readoutIntervalIndex);

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    settings.setValue("SHT4x/Readout_Interval_Index",readoutIntervalIndex);

    if(sht4xEnable)
    {
        readOutSHT4x();
        readOutIntervalCount=0;
        bWaitReadOutResponse=false;
        waitReadOutResponseCount=0;
    }

}


void MainWindow::sht4xCommandSlelectChange(int index)
{
    qDebug() << "[MainWindow]sht4xCommandSlelectChange() index:"<<index;

    sht4xCommandIndex=infoSHT4xComboBox[0]->currentIndex();

    if(sht4xCommandIndex<2)
    {
        if(heatingTemp)
        {
            heatingTemp->setText("Heating Temp: 0");
        }

        if(heatingHumidity)
        {
            heatingHumidity->setText("Hearting Humidity: 0");

        }
    }

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    settings.setValue("SHT4x/CommandType",sht4xCommandIndex);

    if(sht4xEnable)
    {
        readOutSHT4x();
        readOutIntervalCount=0;
        bWaitReadOutResponse=false;
        waitReadOutResponseCount=0;
    }
}

void MainWindow::regSlelectChange(int index)
{
    qDebug() << "[MainWindow]regSlelectChange() index:"<<index;

    if(ui->tabWidget->currentIndex()==0)
    {
        uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
        uint8_t RegAddress=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),0)->text()).toInt(NULL,16);
        if(RegAddress==0x08)
        {
            RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup3Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup3Offset);
            RegValue|=index<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup3Offset;
        }
        else if(RegAddress==0x0B||RegAddress==0x0C||RegAddress==0x0D||RegAddress==0x0E)
        {
            RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset);
            RegValue|=index<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset;
        }
        QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
        ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText(value);

        if(RegAddress==0x08)
        {
            if(comboBox->currentIndex()==0)
                descriptionLabel[3]->setText("ASI data MSB location has no offset and is as per standard protocol");
            else
            {

                descriptionLabel[3]->setText(QString::asprintf("ASI data MSB location (TDM mode is slot 0 or I2S, LJ mode is the left and right slot 0)\r\noffset of %d BCLK cycle with respect to standard protocol",comboBox->currentIndex()));
            }
        }
        else if(RegAddress==0x0B||RegAddress==0x0C||RegAddress==0x0D||RegAddress==0x0E)
        {
            if(comboBox->currentIndex()/32==1)
                descriptionLabel[1]->setText(QString::asprintf("TDM is slot %d or I2S, LJ is right slot %d",comboBox->currentIndex(),comboBox->currentIndex()%32));
            else
            {

                descriptionLabel[1]->setText(QString::asprintf("TDM is slot %d or I2S, LJ is left slot %d",comboBox->currentIndex(),comboBox->currentIndex()%32));
            }
        }
    }
    else if(ui->tabWidget->currentIndex()==1)
    {
        uint8_t RegValue=(ui->tableWidget_page2->item(ui->tableWidget_page2->currentRow(),2)->text()).toInt(NULL,16);

        RegValue=RegValue&~(pag2RegOffset[0].btnGrup1Mask<<pag2RegOffset[0].btnGrup1Offset);
        RegValue|=index<<pag2RegOffset[0].btnGrup1Offset;
        QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
        ui->tableWidget_page2->item(ui->tableWidget_page2->currentRow(),2)->setText(value);
    }
    else if(ui->tabWidget->currentIndex()==2)
    {
        uint8_t RegValue=(ui->tableWidget_page3->item(ui->tableWidget_page3->currentRow(),2)->text()).toInt(NULL,16);

        RegValue=RegValue&~(pag3RegOffset[0].btnGrup1Mask<<pag3RegOffset[0].btnGrup1Offset);
        RegValue|=index<<pag3RegOffset[0].btnGrup1Offset;
        QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
        ui->tableWidget_page3->item(ui->tableWidget_page3->currentRow(),2)->setText(value);
    }
    else if(ui->tabWidget->currentIndex()==3)
    {
        uint8_t RegValue=(ui->tableWidget_page4->item(ui->tableWidget_page4->currentRow(),2)->text()).toInt(NULL,16);

        RegValue=RegValue&~(pag4RegOffset[0].btnGrup1Mask<<pag4RegOffset[0].btnGrup1Offset);
        RegValue|=index<<pag4RegOffset[0].btnGrup1Offset;
        QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
        ui->tableWidget_page4->item(ui->tableWidget_page4->currentRow(),2)->setText(value);
    }
}

void MainWindow::regSliderChange(int index)
{
    qDebug() << "[MainWindow]regSliderChange:"<<index;

    uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
    uint8_t RegAddress=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),0)->text()).toInt(NULL,16);
    RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset);
    RegValue|=index<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset;


    ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText("0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper());

    if(RegAddress==0x3E||RegAddress==0x43||RegAddress==0x48||RegAddress==0x4D)
    {
        if(index==0)
            descriptionLabel[1]->setText(QString::asprintf("Digital volume is muted"));
        else
        {

            descriptionLabel[1]->setText(QString::asprintf("Digital volume control is set to %.1f dB",((index-1)*0.5)-100));
        }
    }
    else if(RegAddress==0x40||RegAddress==0x45||RegAddress==0x4A||RegAddress==0x4F)
    {
        if(index==0)
        {
            descriptionLabel[1]->setText(QString::asprintf("No phase calibration"));
        }
        else
        {

            descriptionLabel[1]->setText(QString::asprintf("Phase calibration delay is set to %d cycle of the modulator clock",index));
        }
    }
}

void MainWindow::tableWidgetPage0SelectChange(QItemSelection selected,QItemSelection deselected)
{
    Q_UNUSED(deselected);

    QModelIndexList items ;
    int selectIndex=0;

    items = selected.indexes();
    selectIndex=items[0].row();

    QString value;
    uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),0)->text()).toInt(NULL,16);

    if(page0Manager.getProperty(ui->tableWidget_page0->currentRow())==REG_READ_ONLY)
    {
        value="Read Only:0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
        ui->page0WriteRegBtn->setEnabled(false);
    }
    else
    {
        value="Write REG:0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
        ui->page0WriteRegBtn->setEnabled(true);
    }

    ui->page0WriteRegBtn->setText(value);

    DrawPageComponent(":/Page0_Component.json",selectIndex);

    qDebug() << "[MainWindow]tableWidgetPage0SelectChange selectIndex:"<<selectIndex;

}



void MainWindow::tableWidgetPage2SelectChange(QItemSelection selected,QItemSelection deselected)
{
    Q_UNUSED(deselected);

    QModelIndexList items ;
    int selectIndex=0;

    items = selected.indexes();
    selectIndex=items[0].row();

    uint8_t RegValue=(ui->tableWidget_page2->item(ui->tableWidget_page2->currentRow(),0)->text()).toInt(NULL,16);
    QString value="Write REG:0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();

    ui->page2WriteRegBtn->setText(value);

    DrawPageComponent(":/Page2_Component.json",selectIndex);

    qDebug() << "[MainWindow]tableWidgetPage2SelectChange selectIndex:"<<selectIndex;
}

void MainWindow::tableWidgetPage3SelectChange(QItemSelection selected,QItemSelection deselected)
{
    Q_UNUSED(deselected);

    QModelIndexList items ;
    int selectIndex=0;

    items = selected.indexes();
    selectIndex=items[0].row();

    uint8_t RegValue=(ui->tableWidget_page3->item(ui->tableWidget_page3->currentRow(),0)->text()).toInt(NULL,16);
    QString value="Write REG:0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();

    ui->page3WriteRegBtn->setText(value);

    DrawPageComponent(":/Page3_Component.json", selectIndex);

    qDebug() << "[MainWindow]tableWidgetPage3SelectChange selectIndex:"<<selectIndex;
}

void MainWindow::tableWidgetPage4SelectChange(QItemSelection selected,QItemSelection deselected)
{
    Q_UNUSED(deselected);

    QModelIndexList items ;
    int selectIndex=0;

    items = selected.indexes();
    selectIndex=items[0].row();

    uint8_t RegValue=(ui->tableWidget_page4->item(ui->tableWidget_page4->currentRow(),0)->text()).toInt(NULL,16);
    QString value="Write REG:0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();

    ui->page4WriteRegBtn->setText(value);

    DrawPageComponent(":/Page4_Component.json", selectIndex);

    qDebug() << "[MainWindow]tableWidgetPage4SelectChange selectIndex:"<<selectIndex;
}

void MainWindow::onTabChanged(int index)
{
    qDebug() << "[MainWindow]onTabChanged index:"<<index;

    deletePage();

    if(index==0&&ui->tableWidget_page0->currentRow()>=0)
    {
        DrawPageComponent(":/Page0_Component.json", ui->tableWidget_page0->currentRow());
    }
    else if(index==1&&ui->tableWidget_page2->currentRow()>=0)
    {
        DrawPageComponent(":/Page2_Component.json", ui->tableWidget_page2->currentRow());
    }
    else if(index==2&&ui->tableWidget_page3->currentRow()>=0)
    {
        DrawPageComponent(":/Page3_Component.json", ui->tableWidget_page3->currentRow());
    }
    else if(index==3&&ui->tableWidget_page4->currentRow()>=0)
    {
        DrawPageComponent(":/Page4_Component.json", ui->tableWidget_page4->currentRow());
    }
    else if(index==4)
    {
        createInfo(infoBtnGrup->checkedId());
    }
}

void MainWindow::deletePage()
{
    if(infoNoteEdit)
    {
        delete infoNoteEdit;
        infoNoteEdit = nullptr;
    }

    if(writeConfig)
    {
        disconnect(writeConfig,SIGNAL(clicked()),0,0);
        delete writeConfig;
        writeConfig = nullptr;
    }

    if(clearBtn)
    {
        disconnect(clearBtn,SIGNAL(clicked()),0,0);
        delete clearBtn;
        clearBtn = nullptr;
    }

    if(getTimeBtn)
    {
        disconnect(getTimeBtn,SIGNAL(clicked()),0,0);
        delete getTimeBtn;
        getTimeBtn = nullptr;
    }

    if(singleReadBtn)
    {
        disconnect(singleReadBtn,SIGNAL(clicked()),0,0);
        delete singleReadBtn;
        singleReadBtn = nullptr;
    }

    if(setSHT4xSoftrestBtn)
    {
        disconnect(setSHT4xSoftrestBtn,SIGNAL(clicked()),0,0);
        delete setSHT4xSoftrestBtn;
        setSHT4xSoftrestBtn = nullptr;
    }

    if(readSHT4xSerialNumberBtn)
    {
        disconnect(readSHT4xSerialNumberBtn,SIGNAL(clicked()),0,0);
        delete readSHT4xSerialNumberBtn;
        readSHT4xSerialNumberBtn = nullptr;
    }

    if(tempCalLineEdit)
    {
        delete tempCalLineEdit;
        tempCalLineEdit= nullptr;
    }

    if(humidityCalLineEdit)
    {
        delete humidityCalLineEdit;
        humidityCalLineEdit= nullptr;
    }

    if(saveCalBtn)
    {
        disconnect(saveCalBtn,SIGNAL(clicked()),0,0);
        delete saveCalBtn;
        saveCalBtn = nullptr;
    }

    if(readCalBtn)
    {
        disconnect(readCalBtn,SIGNAL(clicked()),0,0);
        delete readCalBtn;
        readCalBtn = nullptr;
    }

    if(readFromBtn)
    {
        disconnect(readFromBtn,SIGNAL(clicked()),0,0);
        delete readFromBtn;
        readFromBtn = nullptr;
    }

    if(writeToBtn)
    {
        disconnect(writeToBtn,SIGNAL(clicked()),0,0);
        delete writeToBtn;
        writeToBtn = nullptr;
    }

    if(startCheckBox)
    {
        disconnect(startCheckBox,&QCheckBox::checkStateChanged,0,0);
        delete startCheckBox;
        startCheckBox = nullptr;
    }

    if(enableLogCheckBox)
    {
        disconnect(enableLogCheckBox,&QCheckBox::checkStateChanged,0,0);
        delete enableLogCheckBox;
        enableLogCheckBox = nullptr;
    }

    if(baseTemp)
    {
        delete baseTemp;
        baseTemp= nullptr;
    }

    if(baseHumidity)
    {
        delete baseHumidity;
        baseHumidity= nullptr;
    }

    if(heatingTemp)
    {
        delete heatingTemp;
        heatingTemp= nullptr;
    }

    if(heatingHumidity)
    {
        delete heatingHumidity;
        heatingHumidity= nullptr;
    }

    for(int i=0;i<2;i++)
    {
        if(infoSHT4xComboBox[i])
        {
            disconnect(infoSHT4xComboBox[i],SIGNAL(currentIndexChanged(int)),0,0);
            delete infoSHT4xComboBox[i];
            infoSHT4xComboBox[i] = nullptr;
        }
    }

    for(int i=0;i<3;i++)
    {
        if(infoDPS368ComboBox[i])
        {
            disconnect(infoDPS368ComboBox[i],SIGNAL(currentIndexChanged(int)),0,0);
            delete infoDPS368ComboBox[i];
            infoDPS368ComboBox[i] = nullptr;
        }
    }

    for(int i=0;i<9;i++)
    {
        if(infoSBM100ComboBox[i])
        {
            disconnect(infoSBM100ComboBox[i],SIGNAL(currentIndexChanged(int)),0,0);
            delete infoSBM100ComboBox[i];
            infoSBM100ComboBox[i] = nullptr;
        }
    }

    for(int i=0;i<5;i++)
    {
        if(infoSBM100LineEdit[i])
        {
            delete infoSBM100LineEdit[i];
            infoSBM100LineEdit[i]= nullptr;
        }
    }


    if(comboBox)
    {
        disconnect(comboBox,SIGNAL(currentIndexChanged(int)),0,0);
        delete comboBox;
        comboBox = nullptr;
    }

    if(slider)
    {
        disconnect(slider,SIGNAL(valueChanged(int)),0,0);
        delete slider;
        slider = nullptr;
    }

    if(infoSBM100slider)
    {
        disconnect(infoSBM100slider,SIGNAL(valueChanged(int)),0,0);
        delete infoSBM100slider;
        infoSBM100slider = nullptr;
    }

    for(int i=0;i<10;i++)
    {
        if(GroupBox[i])
        {
            delete GroupBox[i];
            GroupBox[i] = nullptr;
        }

        if(btnGrup[i])
        {
            disconnect(btnGrup[i],SIGNAL(buttonClicked(QAbstractButton*)), 0, 0);
            delete btnGrup[i];
            btnGrup[i] = nullptr;
        }
    }

    for(int i=0;i<40;i++)
    {
        if(descriptionLabel[i])
        {
            delete descriptionLabel[i];
            descriptionLabel[i]= nullptr;
        }

        if(infoLineEdit[i])
        {
            delete infoLineEdit[i];
            infoLineEdit[i]= nullptr;
        }

        if(radioButton[i])
        {
            delete radioButton[i];
            radioButton[i]= nullptr;
        }
    }

}

void MainWindow::handleGroupClick(QAbstractButton* button) {

    int id=0;

    if(btnGrup[0])
    {
        id=btnGrup[0]->id(button);

        if (id != -1) {
            qDebug() << "[MainWindow]radiobtnGroup1clicked:"<<btnGrup[0]->checkedId();

            if(ui->tabWidget->currentIndex()==0)
            {

                uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
                RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset);
                RegValue|=(btnGrup[0]->checkedId())<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset;

                QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
                ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText(value);
            }
        }
    }

    if(btnGrup[1])
    {
        id=btnGrup[1]->id(button);

        if (id != -1) {
            qDebug() << "[MainWindow]radiobtnGroup2clicked:"<<btnGrup[1]->checkedId();
            if(ui->tabWidget->currentIndex()==0)
            {

                uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
                RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup2Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup2Offset);
                RegValue|=(btnGrup[1]->checkedId())<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup2Offset;

                QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
                ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText(value);
            }
        }
    }

    if(btnGrup[2])
    {
        id=btnGrup[2]->id(button);

        if (id != -1) {
            qDebug() << "[MainWindow]radiobtnGroup3clicked:"<<btnGrup[2]->checkedId();
            if(ui->tabWidget->currentIndex()==0)
            {

                uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
                RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup3Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup3Offset);
                RegValue|=(btnGrup[2]->checkedId())<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup3Offset;

                QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
                ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText(value);
        }
        }
    }

    if(btnGrup[3])
    {
        id=btnGrup[3]->id(button);

        if (id != -1) {
            qDebug() << "[MainWindow]radiobtnGroup4clicked:"<<btnGrup[3]->checkedId();
            if(ui->tabWidget->currentIndex()==0)
            {

                uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
                RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup4Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup4Offset);
                RegValue|=(btnGrup[3]->checkedId())<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup4Offset;

                QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
                ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText(value);

            }
        }
    }

    if(btnGrup[4])
    {
        id=btnGrup[4]->id(button);

        if (id != -1) {
            qDebug() << "[MainWindow]radiobtnGroup5clicked:"<<btnGrup[4]->checkedId();

            uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
            RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup5Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup5Offset);
            RegValue|=(btnGrup[4]->checkedId())<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup5Offset;

            QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
            ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText(value);

        }
    }

    if(btnGrup[5])
    {
        id=btnGrup[5]->id(button);

        if (id != -1) {
            qDebug() << "[MainWindow]radiobtnGroup6clicked:"<<btnGrup[5]->checkedId();

            uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
            RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup6Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup6Offset);
            RegValue|=(btnGrup[5]->checkedId())<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup6Offset;

            QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
            ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText(value);

        }
    }
}

void MainWindow::DrawPageComponent(const QString& filename, int memberIndex)
{
    qDebug() << "[MainWindow]DrawPageComponent filename:"<<filename<<" memberIndex:"<<memberIndex;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[MainWindow]Unable to open file";
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[MainWindow]JSON parsing error:" << parseError.errorString();
        qWarning() << "[MainWindow]Error byte offset:" << parseError.offset;

            int lineNumber = 1;
            int column = 1;
            for (int i = 0; i < parseError.offset && i < jsonData.size(); ++i) {
                if (jsonData[i] == '\n') {
                    lineNumber++;
                    column = 1;
                } else {
                    column++;
                }
            }

            qWarning() << "[MainWindow]"<<QString("The error occurred at approximately line %1, character %2").arg(lineNumber).arg(column);
            return;
    }

    QJsonObject root = doc.object();

    QString regKey;
    QJsonArray members = root.value("Member").toArray();
    if(ui->tabWidget->currentIndex()==0)
    {
        if (memberIndex < 0 || memberIndex >= members.size()) {
            qWarning() << "[MainWindow]Member index out of range";
            return;
        }

        regKey = members[memberIndex].toString();
    }
    else
    {
        regKey = members[0].toString();
    }
    qWarning() << "[MainWindow]regKey:"<<regKey;

    QJsonArray regArray = root.value(regKey).toArray();
    qWarning() << "[MainWindow]regArray.size():"<<regArray.size();
    deletePage();

    int radiobuttonOffset=0;
    int RegValue=0;

    regOffset[memberIndex].rowIndex=memberIndex;

    if(ui->tabWidget->currentIndex()==0)
    {
        RegValue=(ui->tableWidget_page0->item(memberIndex,2)->text()).toInt(NULL,16);
    }
    else if(ui->tabWidget->currentIndex()==1)
    {
        RegValue=(ui->tableWidget_page2->item(memberIndex,2)->text()).toInt(NULL,16);
    }
    else if(ui->tabWidget->currentIndex()==2)
    {
        RegValue=(ui->tableWidget_page3->item(memberIndex,2)->text()).toInt(NULL,16);

    }
    else if(ui->tabWidget->currentIndex()==3)
    {
        RegValue=(ui->tableWidget_page4->item(memberIndex,2)->text()).toInt(NULL,16);
    }

    qDebug() << "[MainWindow]tab Index:"<<ui->tabWidget->currentIndex();
    qDebug() << "[MainWindow]row Index:"<<memberIndex<<" RegValue:"<<RegValue;

    for(int group=0;group<regArray.size();group++)
    {
        QJsonObject item = regArray[group].toObject();

        int GroupType = item.value("Type").toVariant().toInt();
        int mask = item.value("Mask").toVariant().toInt();
        int offset = item.value("Offset").toVariant().toInt();
        int checkRegValue=0;

        if(group==0)
        {
            if(ui->tabWidget->currentIndex()==0)
            {
                regOffset[memberIndex].btnGrup1Offset=offset;
                regOffset[memberIndex].btnGrup1Mask=mask;
                checkRegValue=(RegValue&(regOffset[memberIndex].btnGrup1Mask<<regOffset[memberIndex].btnGrup1Offset))>>regOffset[memberIndex].btnGrup1Offset;
            }
            else if(ui->tabWidget->currentIndex()==1)
            {
                  pag2RegOffset[0].btnGrup1Offset=offset;
                  pag2RegOffset[0].btnGrup1Mask=mask;
                  checkRegValue=(RegValue&(pag2RegOffset[0].btnGrup1Mask<<pag2RegOffset[0].btnGrup1Offset))>>pag2RegOffset[0].btnGrup1Offset;
            }
            else if(ui->tabWidget->currentIndex()==2)
            {
                  pag3RegOffset[0].btnGrup1Offset=offset;
                  pag3RegOffset[0].btnGrup1Mask=mask;
                  checkRegValue=(RegValue&(pag3RegOffset[0].btnGrup1Mask<<pag3RegOffset[0].btnGrup1Offset))>>pag3RegOffset[0].btnGrup1Offset;
            }
            else if(ui->tabWidget->currentIndex()==3)
            {
                  pag4RegOffset[0].btnGrup1Offset=offset;
                  pag4RegOffset[0].btnGrup1Mask=mask;
                  checkRegValue=(RegValue&(pag4RegOffset[0].btnGrup1Mask<<pag4RegOffset[0].btnGrup1Offset))>>pag4RegOffset[0].btnGrup1Offset;
            }
        }
        else if(group==1)
        {
            regOffset[memberIndex].btnGrup2Offset=offset;
            regOffset[memberIndex].btnGrup2Mask=mask;
            checkRegValue=(RegValue&(regOffset[memberIndex].btnGrup2Mask<<regOffset[memberIndex].btnGrup2Offset))>>regOffset[memberIndex].btnGrup2Offset;
        }
        else if(group==2)
        {
            regOffset[memberIndex].btnGrup3Offset=offset;
            regOffset[memberIndex].btnGrup3Mask=mask;
            checkRegValue=(RegValue&(regOffset[memberIndex].btnGrup3Mask<<regOffset[memberIndex].btnGrup3Offset))>>regOffset[memberIndex].btnGrup3Offset;
        }
        else if(group==3)
        {
            regOffset[memberIndex].btnGrup4Offset=offset;
            regOffset[memberIndex].btnGrup4Mask=mask;
            checkRegValue=(RegValue&(regOffset[memberIndex].btnGrup4Mask<<regOffset[memberIndex].btnGrup4Offset))>>regOffset[memberIndex].btnGrup4Offset;
        }
        else if(group==4)
        {
            regOffset[memberIndex].btnGrup5Offset=offset;
            regOffset[memberIndex].btnGrup5Mask=mask;
            checkRegValue=(RegValue&(regOffset[memberIndex].btnGrup5Mask<<regOffset[memberIndex].btnGrup5Offset))>>regOffset[memberIndex].btnGrup5Offset;
        }
        else if(group==5)
        {
            regOffset[memberIndex].btnGrup6Offset=offset;
            regOffset[memberIndex].btnGrup6Mask=mask;
            checkRegValue=(RegValue&(regOffset[memberIndex].btnGrup6Mask<<regOffset[memberIndex].btnGrup6Offset))>>regOffset[memberIndex].btnGrup6Offset;
        }
        else if(group==6)
        {
            regOffset[memberIndex].btnGrup7Offset=offset;
            regOffset[memberIndex].btnGrup7Mask=mask;
            checkRegValue=(RegValue&(regOffset[memberIndex].btnGrup7Mask<<regOffset[memberIndex].btnGrup7Offset))>>regOffset[memberIndex].btnGrup7Offset;
        }
        else if(group==7)
        {
            regOffset[memberIndex].btnGrup8Offset=offset;
            regOffset[memberIndex].btnGrup8Mask=mask;
            checkRegValue=(RegValue&(regOffset[memberIndex].btnGrup8Mask<<regOffset[memberIndex].btnGrup8Offset))>>regOffset[memberIndex].btnGrup8Offset;
        }

        if(GroupType==0)
        {
            int GropuBox_x = item.value("GroupBox_X").toVariant().toInt();
            int GropuBox_y = item.value("GroupBox_Y").toVariant().toInt();
            int GropuBox_w = item.value("GroupBox_W").toVariant().toInt();
            int GropuBox_h = item.value("GroupBox_H").toVariant().toInt();
            QString GroupBoxTitle = item.value("GroupBox_Title").toVariant().toString();

            QString LabelString = item.value("Description_StringValue").toVariant().toString();
            int Description_0_X = item.value("Description_X").toVariant().toInt();
            int Description_0_Y = item.value("Description_Y").toVariant().toInt();
            int Description_0_W = item.value("Description_W").toVariant().toInt();
            int Description_0_H = item.value("Description_H").toVariant().toInt();

            int NumberOfButton=item.value("NumberOfRadioButton").toVariant().toInt();

            GroupBox[group] = new QGroupBox(this);
            GroupBox[group]->setStyleSheet(QStringLiteral("QGroupBox{border:1px solid white;font-weight: bold;font-size: 10pt;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top left;padding:0.3px;}"));
            GroupBox[group]->setGeometry(GropuBox_x,GropuBox_y,GropuBox_w,GropuBox_h);
            GroupBox[group]->setTitle(GroupBoxTitle);
            GroupBox[group]->show();

            descriptionLabel[group] = new QLabel(this);
            descriptionLabel[group]->setGeometry(Description_0_X,Description_0_Y,Description_0_W,Description_0_H);
            descriptionLabel[group]->setText(LabelString);
            descriptionLabel[group]->show();


            btnGrup[group] = new QButtonGroup(this);

            for(int radioButtonNumber=0;radioButtonNumber<NumberOfButton;radioButtonNumber++)
            {
                QString radioStringValue = item.value(QString("RadioButton_%1_Option").arg(radioButtonNumber)).toVariant().toString();
                int RadioButton_X = item.value(QString("RadioButton_%1_X").arg(radioButtonNumber)).toVariant().toInt();
                int RadioButton_Y = item.value(QString("RadioButton_%1_Y").arg(radioButtonNumber)).toVariant().toInt();
                int RadioButton_W = item.value(QString("RadioButton_%1_W").arg(radioButtonNumber)).toVariant().toInt();
                int RadioButton_H = item.value(QString("RadioButton_%1_H").arg(radioButtonNumber)).toVariant().toInt();
                int RadioButton_ID = item.value(QString("RadioButton_%1_ID").arg(radioButtonNumber)).toVariant().toInt();
                int RadioButton_ReadyOnly = item.value(QString("RadioButton_%1_Read_Only").arg(radioButtonNumber)).toVariant().toInt();


                radioButton[radiobuttonOffset]= new QRadioButton(this);

                radioButton[radiobuttonOffset]->setStyleSheet(R"(
                    QRadioButton {
                         color: white;
                         spacing: 8px;
                    }
                    QRadioButton::indicator {
                         width: 16px;
                         height: 16px;
                    }
                    QRadioButton::indicator:unchecked {
                         image: url(:/radio_unchecked.ico);
                    }
                    QRadioButton::indicator:checked {
                         image: url(:/radio_checked.ico);
                    }
                )");

                if(RadioButton_ReadyOnly)
                {
                    radioButton[radiobuttonOffset]->setAttribute(Qt::WA_TransparentForMouseEvents);
                    radioButton[radiobuttonOffset]->setFocusPolicy(Qt::NoFocus);
                }

                radioButton[radiobuttonOffset]->setGeometry(RadioButton_X,RadioButton_Y,RadioButton_W,RadioButton_H);
                radioButton[radiobuttonOffset]->setText(radioStringValue);
                radioButton[radiobuttonOffset]->show();

                btnGrup[group]->addButton(radioButton[radiobuttonOffset]);
                btnGrup[group]->setId(radioButton[radiobuttonOffset],RadioButton_ID);
                radiobuttonOffset++;

            }

            if(memberIndex==13)
            {
                if(group==0)
                {
                    if(checkRegValue>=9)
                        checkRegValue=9;
                }
                else
                {
                    if(checkRegValue>=13)
                        checkRegValue=13;
                }
            }
            else if(memberIndex==47)
            {
                if(checkRegValue!=4 && checkRegValue!=6 && checkRegValue!=7)
                    checkRegValue=8;
            }


            QAbstractButton* targetButton = btnGrup[group]->button(checkRegValue);

            if(targetButton)
            {
                btnGrup[group]->button(checkRegValue)->setChecked(true);
            }
            else
            {
                QString errMsg =  "[MainWindow]REG:"+ui->tableWidget_page0->item(memberIndex,0)->text()+"value is illegal"+"(Button Group)"+QString::number(group);
                qDebug() << errMsg;
                QMessageBox::information(this,NULL, errMsg);
            }

            connect(btnGrup[group],SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(handleGroupClick(QAbstractButton*)));
        }
        else if(GroupType==1)
        {
            QString LabelString = item.value("Description_StringValue").toVariant().toString();
            int Description_X = item.value("Description_X").toVariant().toInt();
            int Description_Y = item.value("Description_Y").toVariant().toInt();
            int Description_W = item.value("Description_W").toVariant().toInt();
            int Description_H = item.value("Description_H").toVariant().toInt();

            int ComboBox_X = item.value("ComboBox_0_X").toVariant().toInt();
            int ComboBox_Y = item.value("ComboBox_0_Y").toVariant().toInt();
            int ComboBox_W = item.value("ComboBox_0_W").toVariant().toInt();
            int ComboBox_H = item.value("ComboBox_0_H").toVariant().toInt();

            int ComboBox_MinValue = item.value("ComboBox_MinValue").toVariant().toInt();
            int ComboBox_MaxValue = item.value("ComboBox_MaxValue").toVariant().toInt();

            int Result_Description_X = item.value("Result_Description_X").toVariant().toInt();
            int Result_Description_Y = item.value("Result_Description_Y").toVariant().toInt();
            int Result_Description_W = item.value("Result_Description_W").toVariant().toInt();
            int Result_Description_H = item.value("Result_Description_H").toVariant().toInt();

            descriptionLabel[group] = new QLabel(this);
            descriptionLabel[group]->setGeometry(Description_X,Description_Y,Description_W,Description_H);
            descriptionLabel[group]->setText(LabelString);
            descriptionLabel[group]->show();

            comboBox = new QComboBox(this);
            comboBox->setStyleSheet("background-color: rgb(0, 0, 0);");

            for(int i=ComboBox_MinValue;i<ComboBox_MaxValue;i++)
            {
                if(ui->tabWidget->currentIndex()!=0)
                {
                    QString value="0x"+QString("%1").arg(i, 2, 16, QLatin1Char('0')).toUpper();
                    comboBox->addItem(value);
                }
                else
                {
                    comboBox->addItem(QLocale().toString(i));
                }
            }

            comboBox->setCurrentIndex(checkRegValue);
            comboBox->setGeometry(ComboBox_X,ComboBox_Y,ComboBox_W,ComboBox_H);
            if(ui->tabWidget->currentIndex()!=0)
            {
                comboBox->setMaxVisibleItems(30);
            }
            comboBox->show();

            descriptionLabel[group+1] = new QLabel(this);
            descriptionLabel[group+1]->setGeometry(Result_Description_X,Result_Description_Y,Result_Description_W,Result_Description_H);
            if(ui->tabWidget->currentIndex()==0)
            {
                uint8_t RegAddress=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),0)->text()).toInt(NULL,16);

                if(RegAddress==0x08)
                {
                    if(comboBox->currentIndex()==0)
                        descriptionLabel[group+1]->setText("ASI data MSB location has no offset and is as per standard protocol");
                    else
                    {

                        descriptionLabel[group+1]->setText(QString::asprintf("ASI data MSB location (TDM mode is slot 0 or I2S, LJ mode is the left and right slot 0)\r\noffset of %d BCLK cycle with respect to standard protocol",comboBox->currentIndex()));
                    }
                }
                else if(RegAddress==0x0B||RegAddress==0x0C||RegAddress==0x0D||RegAddress==0x0E)
                {
                    if(comboBox->currentIndex()/32==1)
                        descriptionLabel[group+1]->setText(QString::asprintf("TDM is slot %d or I2S, LJ is right slot %d",comboBox->currentIndex(),comboBox->currentIndex()%32));
                    else
                    {

                        descriptionLabel[group+1]->setText(QString::asprintf("TDM is slot %d or I2S, LJ is left slot %d",comboBox->currentIndex(),comboBox->currentIndex()%32));
                    }
                }

                descriptionLabel[group+1]->show();
            }

            connect(comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(regSlelectChange(int)));

        }
        else if(GroupType==2)
        {
            QString LabelString = item.value("Description_StringValue").toVariant().toString();
            int Description_0_X = item.value("Description_X").toVariant().toInt();
            int Description_0_Y = item.value("Description_Y").toVariant().toInt();
            int Description_0_W = item.value("Description_W").toVariant().toInt();
            int Description_0_H = item.value("Description_H").toVariant().toInt();

            int Slider_X = item.value("Slider_0_X").toVariant().toInt();
            int Slider_Y = item.value("Slider_0_Y").toVariant().toInt();
            int Slider_W = item.value("Slider_0_W").toVariant().toInt();
            int Slider_H = item.value("Slider_0_H").toVariant().toInt();
            int Slider_MinValue = item.value("Slider_MinValue").toVariant().toInt();
            int Slider_MaxValue = item.value("Slider_MaxValue").toVariant().toInt();

            int Result_Description_X = item.value("Result_Description_X").toVariant().toInt();
            int Result_Description_Y = item.value("Result_Description_Y").toVariant().toInt();
            int Result_Description_W = item.value("Result_Description_W").toVariant().toInt();
            int Result_Description_H = item.value("Result_Description_H").toVariant().toInt();


            descriptionLabel[group] = new QLabel(this);
            descriptionLabel[group]->setGeometry(Description_0_X,Description_0_Y,Description_0_W,Description_0_H);
            descriptionLabel[group]->setText(LabelString);
            descriptionLabel[group]->show();

            //int checkRegValue=(RegValue&(regOffset[memberIndex].btnGrup1Mask<<regOffset[memberIndex].btnGrup1Offset))>>regOffset[memberIndex].btnGrup1Offset;
            uint8_t RegAddress=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),0)->text()).toInt(NULL,16);

            slider = new QSlider(Qt::Horizontal,this);
            slider->setRange(Slider_MinValue,Slider_MaxValue);

            slider->setGeometry(Slider_X,Slider_Y,Slider_W,Slider_H);
            slider->setSliderPosition(checkRegValue);
            slider->setTickInterval(10);
            slider->setTickPosition(QSlider::TicksBelow);
            slider->show();

            descriptionLabel[group+1] = new QLabel(this);
            descriptionLabel[group+1]->setGeometry(Result_Description_X,Result_Description_Y,Result_Description_W,Result_Description_H);
            if(RegAddress==0x3E||RegAddress==0x43||RegAddress==0x48||RegAddress==0x4D)
            {
                if(checkRegValue==0)
                    descriptionLabel[group+1]->setText(QString::asprintf("Digital volume is muted"));
                else
                {

                    descriptionLabel[group+1]->setText(QString::asprintf("Digital volume control is set to %.1f dB",((checkRegValue-1)*0.5)-100));
                }
            }
            else if(RegAddress==0x40||RegAddress==0x45||RegAddress==0x4A||RegAddress==0x4F)
            {
                if(checkRegValue==0)
                {
                    descriptionLabel[1]->setText(QString::asprintf("No phase calibration"));
                }
                else
                {

                    descriptionLabel[1]->setText(QString::asprintf("Phase calibration delay is set to %d cycle of the modulator clock",checkRegValue));
                }
            }
            descriptionLabel[group+1]->show();

            connect(slider,SIGNAL(valueChanged(int)),this,SLOT(regSliderChange(int)));
        }
    }

}

int MainWindow::fromBCD(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t MainWindow::toBCD(int val) {
    return static_cast<uint8_t>(((val / 10) << 4) | (val % 10));
}

void MainWindow::dateTimeToBCD(uint8_t *bcd,const QDateTime& dt) {

    bcd[0]=(toBCD(dt.date().year() % 100));  // YY
    bcd[1]=(toBCD(dt.date().month()));       // MM
    bcd[2]=(toBCD(dt.date().day()));         // DD
    bcd[3]=(toBCD(dt.time().hour()));        // HH
    bcd[4]=(toBCD(dt.time().minute()));      // mm
    bcd[5]=(toBCD(dt.time().second()));      // SS
}

QDateTime MainWindow::bcdToDateTime(uint8_t *bcd) {

    int year = fromBCD(static_cast<uint8_t>(bcd[0])) + 2000;
    int month = fromBCD(static_cast<uint8_t>(bcd[1]));
    int day = fromBCD(static_cast<uint8_t>(bcd[2]));
    int hour = fromBCD(static_cast<uint8_t>(bcd[3]));
    int minute = fromBCD(static_cast<uint8_t>(bcd[4]));
    int second = fromBCD(static_cast<uint8_t>(bcd[5]));

    QDate date(year, month, day);
    QTime time(hour, minute, second);

    if (!date.isValid() || !time.isValid())
        return QDateTime();

    return QDateTime(date, time);
}

void MainWindow::clearBtn_clicked()
{
    qDebug() << "[MainWindow]clearBtn_clicked";
    infoNoteEdit->clear();
}

void MainWindow::rotateInfoRecordFileIfNeeded() {
    QSettings settings("AppConfig.ini", QSettings::IniFormat);

    needRecordInfo = settings.value("INFO_Record/Enable", "0").toInt();

    if(needRecordInfo==0)
        return;

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    if (today == m_curDate && m_file.isOpen()) return;
    m_curDate = today;
    QDir().mkpath(m_logDir);
    if (m_file.isOpen()) m_file.close();
    QString fn = QString("%1/Info_%2.%3").arg(m_logDir, m_curDate, "csv");
    m_file.setFileName(fn);
    m_file.open(QIODevice::Append | QIODevice::Text);
}

void MainWindow::recordWriteInfo()
{

    rotateInfoRecordFileIfNeeded();

    QTextStream out(&m_file);

    // 寫入當下時間
    QDateTime now = QDateTime::currentDateTime();
    out <<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>," <<"Recore Start Time:" << now.toString("yyyy-MM-dd hh:mm:ss,") <<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";

    // 寫入表頭
    out << "Field,Original Value,New Value\n";


    for(int i=0;i<26;i++)
    {
        QString label = descriptionLabel[i]->text();
        QString oldVal = oldValueList.at(i);
        QString newVal = infoLineEdit[i]->text();
        out << QString("\"%1\",\"%2\",\"%3\"\n").arg(label, oldVal, newVal);
    }

    m_file.close();
}

void MainWindow::rotateNoteRecordFileIfNeeded() {
    QSettings settings("AppConfig.ini", QSettings::IniFormat);

    needRecordInfo = settings.value("INFO_Record/Enable", "0").toInt();

    if(needRecordInfo==0)
        return;

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    if (today == m_curDate && m_file.isOpen()) return;
    m_curDate = today;
    QDir().mkpath(m_logDir);
    if (m_file.isOpen()) m_file.close();
    QString fn = QString("%1/Note_%2.%3").arg(m_logDir, m_curDate, "csv");
    m_file.setFileName(fn);
    m_file.open(QIODevice::Append | QIODevice::Text);
}

void MainWindow::recordWriteNote()
{

    rotateNoteRecordFileIfNeeded();

    QTextStream out(&m_file);

    // 寫入當下時間
    QDateTime now = QDateTime::currentDateTime();
    out <<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>," <<"Recore Start Time:" << now.toString("yyyy-MM-dd hh:mm:ss,") <<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";

    // 寫入表頭
    out << "Field,Original Value,New Value\n";

    QString label = "Note:";
    QString oldVal = oldValueList.at(0);
    QString newVal = infoNoteEdit->toPlainText();
    out << QString("\"%1\",\"%2\",\"%3\"\n").arg(label, oldVal, newVal);

    m_file.close();
}

void MainWindow::rotateHubInfoRecordFileIfNeeded()
{
    QSettings settings("AppConfig.ini", QSettings::IniFormat);

    needRecordInfo = settings.value("INFO_Record/Enable", "0").toInt();

    if(needRecordInfo==0)
        return;

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    if (today == m_curDate && m_file.isOpen()) return;
    m_curDate = today;
    QDir().mkpath(m_logDir);
    if (m_file.isOpen()) m_file.close();
    QString fn = QString("%1/HubInfo_%2.%3").arg(m_logDir, m_curDate, "csv");
    m_file.setFileName(fn);
    m_file.open(QIODevice::Append | QIODevice::Text);
}

void MainWindow::recordWriteHubInfo()
{
    rotateHubInfoRecordFileIfNeeded();

    QTextStream out(&m_file);

    // 寫入當下時間
    QDateTime now = QDateTime::currentDateTime();
    out <<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>," <<"Recore Start Time:" << now.toString("yyyy-MM-dd hh:mm:ss,") <<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";

    // 寫入表頭
    out << "Field,Original Value,New Value\n";


    for(int i=0;i<5;i++)
    {
        QString label = descriptionLabel[i]->text();
        QString oldVal = oldValueList.at(i);
        QString newVal = infoLineEdit[i]->text();
        out << QString("\"%1\",\"%2\",\"%3\"\n").arg(label, oldVal, newVal);
    }

    m_file.close();
}


void MainWindow::rotateSHT4xSeparateCheckRecordFileIfNeeded() {

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString serial= "0x"+QString("%1").arg(sht4xSerialNumber, 4, 16, QLatin1Char('0')).toUpper();
    bool fileExists;
    if (today == sht4x_m_curDate && sht4x_m_file.isOpen()) return;
    sht4x_m_curDate = today;
    QDir().mkpath(sht4x_m_logDir);
    if (sht4x_m_file.isOpen()) sht4x_m_file.close();
    QString fn = QString("%1/sht4x_separate_check_log_%2_%3.%4").arg(sht4x_m_logDir,serial, sht4x_m_curDate, "csv");
    sht4x_m_file.setFileName(fn);
    fileExists=sht4x_m_file.exists();
    sht4x_m_file.open(QIODevice::Append | QIODevice::Text);

    if(!fileExists)
    {
        QTextStream out(&sht4x_m_file);
        // 寫入表頭
        out << "serial number,type of readout,timestamp,temperature, humidity\n";
    }
}

void MainWindow::rotateSHT4xRecordFileIfNeeded() {

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString serial= "0x"+QString("%1").arg(sht4xSerialNumber, 4, 16, QLatin1Char('0')).toUpper();
    bool fileExists;
    if (today == sht4x_m_curDate && sht4x_m_file.isOpen()) return;
    sht4x_m_curDate = today;
    QDir().mkpath(sht4x_m_logDir);
    if (sht4x_m_file.isOpen()) sht4x_m_file.close();
    QString fn = QString("%1/sht4x_log_%2_%3.%4").arg(sht4x_m_logDir,serial, sht4x_m_curDate, "csv");
    sht4x_m_file.setFileName(fn);
    fileExists=sht4x_m_file.exists();
    sht4x_m_file.open(QIODevice::Append | QIODevice::Text);

    if(!fileExists)
    {
        QTextStream out(&sht4x_m_file);
        // 寫入表頭
        out << "serial number,type of readout,timestamp,temperature, humidity\n";
    }
}

void MainWindow::recordSHT4x(int type)
{

    if(sht4xEnableLog==0)
        return;

    rotateSHT4xRecordFileIfNeeded();

    QString serial,cmdType,logTime,t,H;

    if(type==0)
    {
        serial= "0x"+QString("%1").arg(sht4xSerialNumber, 4, 16, QLatin1Char('0')).toUpper();
        cmdType= "0x"+QString("%1").arg(getSHT4xCMD(sht4xCommandIndex), 2, 16, QLatin1Char('0')).toUpper();
        logTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz 'ms'");
        t=QString::number(temp/1000.0, 'f', 3)/*QLocale().toString(temp/1000.0)*/;
        H=QString::number(humidity/1000.0, 'f', 3)/*QLocale().toString(humidity/1000.0)*/;
    }
    else if(type==1)
    {
        serial= "0x"+QString("%1").arg(sht4xSerialNumber, 4, 16, QLatin1Char('0')).toUpper();
        cmdType ="0x94";
        logTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz 'ms'");
        t="";
        H="";
    }

    QTextStream out(&sht4x_m_file);


    out << QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\"\n").arg(serial, cmdType, logTime,t,H);

    sht4x_m_file.close();
}

void MainWindow::recordSHT4xNotify()
{

    if(sht4xEnableLog==0)
        return;

    if(sht4xLogType==1)
        rotateSHT4xSeparateCheckRecordFileIfNeeded();
    else
        rotateSHT4xRecordFileIfNeeded();

    QString serial,cmdType,logTime,t,H;


    serial= "0x"+QString("%1").arg(sht4xSerialNumber, 4, 16, QLatin1Char('0')).toUpper();
    cmdType= "0xFD";
    logTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz 'ms'");
    t=QString::number(notifyTemp/1000.0, 'f', 3)/*QLocale().toString(notifyTemp/1000.0)*/;
    H=QString::number(notifyHumidity/1000.0, 'f', 3)/*QLocale().toString(notifyHumidity/1000.0)*/;

    QTextStream out(&sht4x_m_file);


    out << QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\"\n").arg(serial, cmdType, logTime,t,H);

    sht4x_m_file.close();
}



void MainWindow::rotateDSP368RecordFileIfNeeded() {

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString serial= QString::fromLatin1(info.systemSerialnumber);
    bool fileExists;
    if (today == DSP368_m_curDate && DSP368_m_file.isOpen()) return;
    DSP368_m_curDate = today;
    QDir().mkpath(sht4x_m_logDir);
    if (DSP368_m_file.isOpen()) DSP368_m_file.close();
    QString fn = QString("%1/DSP368_log_%2_%3.%4").arg(sht4x_m_logDir,serial, DSP368_m_curDate, "csv");
    DSP368_m_file.setFileName(fn);
    fileExists=DSP368_m_file.exists();
    DSP368_m_file.open(QIODevice::Append | QIODevice::Text);

    if(!fileExists)
    {
        QTextStream out(&DSP368_m_file);

        out << "serial number,temp oversample,pressure oversample,timestamp,temperature, pressure\n";
    }
}

QString MainWindow::getOSR(int index)
{
    QString tempStr;

    if(index==0)
        tempStr="0000 - single";
    else if(index==1)
        tempStr="0001 - 2 times";
    else if(index==2)
        tempStr="0010 - 4 times";
    else if(index==3)
        tempStr="0011 - 8 times";
    else if(index==4)
        tempStr="0100 - 16 times";
    else if(index==5)
        tempStr="0101 - 32 times";
    else if(index==6)
        tempStr="0110 - 64 times";
    else if(index==7)
        tempStr="0111 - 128 times";

    return tempStr;
}

void MainWindow::recordDSP368(int type)
{

    if(dsp368EnableLog==0)
        return;

    rotateDSP368RecordFileIfNeeded();

    QString serial,TOSR,POSR,logTime,t,p;

    if(type==0)
    {
        serial= QString::fromLatin1(info.systemSerialnumber);
        TOSR=getOSR(dsp368TempOSRIndex);
        POSR=getOSR(dsp368PressureOSRIndex);
        logTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz 'ms'");
        t=QString::number(dsp368_temp, 'f', 3)/*QLocale().toString(dsp368_temp)*/;
        p=QString::number(dsp368_pressure, 'f', 3)/*QLocale().toString(dsp368_pressure)*/;
    }
    else if(type==1)
    {
        serial= QString::fromLatin1(info.systemSerialnumber);
        TOSR ="Soft Reset";
        POSR ="Soft Reset";
        logTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz 'ms'");
        t="";
        p="";
    }

    QTextStream out(&DSP368_m_file);


    out << QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\"\n").arg(serial, TOSR, POSR, logTime,t,p);

    DSP368_m_file.close();
}


void MainWindow::writeConfig_clicked()
{
    uint8_t pageBuf[140];
    QByteArray tempBuf;
    bool ok;

    qDebug() << "[MainWindow]writeConfig_clicked";

    if(!IsConnect)
    {
        QMessageBox::information(NULL, "MessageBox", "device not connect!!");
        return;
    }

    if(infoBtnGrup->checkedId()==1)
    {
        DeviceInfo  writeInfo;

        memset(pageBuf,0x00,130);

        pageBuf[0]=BLOCK_ID_INFO1;

        tempBuf=infoLineEdit[0]->text().toLatin1();
        memcpy(writeInfo.cartridge1ModelNumber,tempBuf.constData(),8);

        tempBuf=infoLineEdit[1]->text().toLatin1();
        memcpy(writeInfo.cartridge1VersionLetter,tempBuf.constData(),1);

        tempBuf=infoLineEdit[2]->text().toLatin1();
        memcpy(writeInfo.cartridge1SerialNumber,tempBuf.constData(),16);

        writeInfo.cartridge1Sensitivity = infoLineEdit[3]->text().trimmed().toFloat();

        writeInfo.cartridge1ReferenceFrequency = infoLineEdit[4]->text().trimmed().toFloat();

        writeInfo.cartridge1UnitsCode = infoLineEdit[5]->text().trimmed().toUInt();

        writeInfo.cartridge1FrequencyRangeMin=static_cast<int16_t>(infoLineEdit[6]->text().trimmed().toShort());

        writeInfo.cartridge1FrequencyRangeMax=static_cast<int16_t>(infoLineEdit[7]->text().trimmed().toShort());

        writeInfo.cartridge1ChannelAssignment= infoLineEdit[8]->text().trimmed().toUInt();


        tempBuf=infoLineEdit[9]->text().trimmed().toLatin1();
        memcpy(writeInfo.cartridge2ModelNumber,tempBuf.constData(),8);

        tempBuf=infoLineEdit[10]->text().trimmed().toLatin1();
        memcpy(writeInfo.cartridge2VersionLetter,tempBuf.constData(),1);

        tempBuf=infoLineEdit[11]->text().trimmed().toLatin1();
        memcpy(writeInfo.cartridge2SerialNumber,tempBuf.constData(),16);

        writeInfo.cartridge2Sensitivity = infoLineEdit[12]->text().trimmed().toFloat();

        writeInfo.cartridge2ReferenceFrequency = infoLineEdit[13]->text().trimmed().toFloat();

        writeInfo.cartridge2UnitsCode = infoLineEdit[14]->text().trimmed().toUInt();

        writeInfo.cartridge2FrequencyRangeMin=static_cast<int16_t>(infoLineEdit[15]->text().trimmed().toShort());

        writeInfo.cartridge2FrequencyRangeMax=static_cast<int16_t>(infoLineEdit[16]->text().trimmed().toShort());

        writeInfo.cartridge2ChannelAssignment= infoLineEdit[17]->text().trimmed().toUInt();

        tempBuf=infoLineEdit[18]->text().trimmed().toLatin1();
        memcpy(writeInfo.systemDigitInterfaceType,tempBuf.constData(),4);

        writeInfo.systemBitClockFrequency = infoLineEdit[19]->text().trimmed().toFloat();

        writeInfo.systemWordLength= infoLineEdit[20]->text().trimmed().toUInt();

        writeInfo.systemSampleRate = infoLineEdit[21]->text().trimmed().toFloat();

        tempBuf=infoLineEdit[22]->text().trimmed().toLatin1();
        memcpy(writeInfo.systemSerialnumber,tempBuf.constData(),16);

        writeInfo.systemSensitivity = infoLineEdit[23]->text().toFloat();

        QString input=infoLineEdit[24]->text().trimmed();

        QDateTime dt = QDateTime::fromString(input,"yyyy-MM-dd HH:mm:ss");

        if(dt.isValid())
        {
            dateTimeToBCD(writeInfo.systemCalibrationDate,dt);
        }
        else
        {
            writeInfo.systemCalibrationDate[0]=0;
            writeInfo.systemCalibrationDate[1]=1;
            writeInfo.systemCalibrationDate[2]=1;
            writeInfo.systemCalibrationDate[3]=0;
            writeInfo.systemCalibrationDate[4]=0;
            writeInfo.systemCalibrationDate[5]=0;
            qDebug() << "time error"<<dt;
        }

        writeInfo.systemManufactuerID = static_cast<uint16_t>(infoLineEdit[25]->text().trimmed().toUInt(&ok,16));

        memcpy(&pageBuf[1],&writeInfo,sizeof(DeviceInfo));

        pageBuf[128+1]=calculateChecksum(&pageBuf[1],128);

        //bRefreshInfoBlock1=true;

        GenCommand(WRITE_EEPROM_BLOCK_CMD, pageBuf,128+2);
    }
    else if(infoBtnGrup->checkedId()==2)
    {
        DeviceInfoNote writeNote;

        memset(pageBuf,0x00,130);
        memset(writeNote.note,0x00,128);

        pageBuf[0]=BLOCK_ID_INFO2;

        QString text = infoNoteEdit->toPlainText();
        QByteArray byteArray = text.toUtf8();

        memcpy(writeNote.note,byteArray.constData(),byteArray.size());

        memcpy(&pageBuf[1],&writeNote,sizeof(DeviceInfoNote));

        pageBuf[128+1]=calculateChecksum(&pageBuf[1],128);

        //bRefreshInfoBlock2=true;

        GenCommand(WRITE_EEPROM_BLOCK_CMD, pageBuf,128+2);


    }
    else if(infoBtnGrup->checkedId()==3)
    {

        if(handle==NULL)
        {
            qDebug() << "[MainWindow]MCP2221 is not connected!!";
            QMessageBox::information(NULL, "MessageBox", "MCP2221 is not connected!!");
            return;
        }

        unsigned int VID = static_cast<uint16_t>(infoLineEdit[0]->text().trimmed().toUInt(&ok,16));
        unsigned int PID = static_cast<uint16_t>(infoLineEdit[1]->text().trimmed().toUInt(&ok,16));

        std::wstring productDescriptor = infoLineEdit[2]->text().trimmed().toStdWString();  // QString → std::wstring
        wchar_t* productDescriptorPtr = const_cast<wchar_t*>(productDescriptor.c_str());

        std::wstring manuFactureDescriptor = infoLineEdit[3]->text().trimmed().toStdWString();  // QString → std::wstring
        wchar_t* manuFactureDescriptorPtr = const_cast<wchar_t*>(manuFactureDescriptor.c_str());

        std::wstring serialnumberDescriptor = infoLineEdit[4]->text().trimmed().toStdWString();  // QString → std::wstring
        wchar_t* serialnumberDescriptorPtr = const_cast<wchar_t*>(serialnumberDescriptor.c_str());

        Mcp2221_SetVidPid(handle, VID, PID);
        Mcp2221_SetProductDescriptor(handle, productDescriptorPtr);
        Mcp2221_SetManufacturerDescriptor(handle, manuFactureDescriptorPtr);
        Mcp2221_SetSerialNumberDescriptor(handle,serialnumberDescriptorPtr);

        Mcp2221_GetVidPid(handle, &mcpVID, &mcpPID);
        Mcp2221_GetManufacturerDescriptor(handle, MfrDescriptor);
        Mcp2221_GetProductDescriptor(handle, ProdDescrip);
        Mcp2221_GetSerialNumberDescriptor(handle, PordSerNum);

        if(wcscmp(productDescriptorPtr,ProdDescrip)==0&&wcscmp(manuFactureDescriptorPtr,MfrDescriptor)==0&&wcscmp(serialnumberDescriptorPtr,PordSerNum)==0&&mcpVID==VID&&mcpPID==PID)
        {
            qDebug() << "[MainWindow]Mcp2221 Write Successed!!";
            recordWriteHubInfo();
            Mcp2221_Reset(handle);
            QMessageBox::information(NULL, "MessageBox", "Write Successed!\r\nreset mcp2221\r\nplease reconnect your comport!");
            infoBtnGrup->button(0)->setChecked(true);
            if(ui->tabWidget->currentIndex()==4)
            {
                onTabChanged(4);
            }
        }
        else
        {
            qDebug() << "[MainWindow]Mcp2221 Write Fail!!";
            QMessageBox::information(NULL, "MessageBox", "Write Fail!!");
        }
    }
}

uint8_t MainWindow::getSHT4xCMD(int index)
{
    uint8_t cmd=0xFD;
    if(index!=-1)
    {
        if(index==0)
        {
            cmd=0xFD;
        }
        else if(index==1)
        {
            cmd=0xF6;
        }
        else if(index==2)
        {
            cmd=0xE0;
        }
        else if(index==3)
        {
            cmd=0x39;
        }
        else if(index==4)
        {
            cmd=0x32;
        }
        else if(index==5)
        {
            cmd=0x2F;
        }
        else if(index==6)
        {
            cmd=0x24;
        }
        else if(index==7)
        {
            cmd=0x1E;
        }
        else if(index==8)
        {
            cmd=0x15;
        }

    }

    return cmd;
}


void MainWindow::onLogStateChanged(Qt::CheckState state)
{
    if(state == Qt::Checked)
    {
        qDebug() << "onLogStateChanged Checked";

        if(infoBtnGrup->checkedId()==4)
        {
            sht4xEnableLog = 1;
            QSettings settings("AppConfig.ini", QSettings::IniFormat);
            settings.setValue("SHT4x/SHT4x_Enable_Log",sht4xEnableLog);
        }
        else if(infoBtnGrup->checkedId()==5)
        {
            dsp368EnableLog = 1;
            QSettings settings("AppConfig.ini", QSettings::IniFormat);
            settings.setValue("DSP368/DSP368_Enable_Log",dsp368EnableLog);
        }


    }
    else
    {
        qDebug() << "onLogStateChanged Unchecked";

        if(infoBtnGrup->checkedId()==4)
        {
            sht4xEnableLog = 0;
            QSettings settings("AppConfig.ini", QSettings::IniFormat);
            settings.setValue("SHT4x/SHT4x_Enable_Log",sht4xEnableLog);
        }
        else if(infoBtnGrup->checkedId()==5)
        {
            dsp368EnableLog = 0;
            QSettings settings("AppConfig.ini", QSettings::IniFormat);
            settings.setValue("DSP368/DSP368_Enable_Log",dsp368EnableLog);
        }

    }
}

void MainWindow::onStateChanged(Qt::CheckState state)
{
    if(state == Qt::Checked)
    {
        qDebug() << "onStateChanged Checked";

        if(infoBtnGrup->checkedId()==4)
        {
            if(sht4xEnable!=1)
            {
                sht4xEnable = 1;

                QSettings settings("AppConfig.ini", QSettings::IniFormat);
                settings.setValue("SHT4x/SHT4x_Enable",sht4xEnable);

                if(sht4xEnable)
                {
                    readOutSHT4x();
                    readOutIntervalCount=0;
                    bWaitReadOutResponse=false;
                    waitReadOutResponseCount=0;
                }
            }


            singleReadBtn->setDisabled(true);
            bSingleRead=false;
        }
        else if(infoBtnGrup->checkedId()==5)
        {
            if(dsp368Enable!=1)
            {
                dsp368Enable = 1;

                QSettings settings("AppConfig.ini", QSettings::IniFormat);
                settings.setValue("DSP368/DSP368_Enable",dsp368Enable);

                if(dsp368Enable)
                {
                    //readOutSHT4x();
                    dsp368ReadOutIntervalCount=0;
                    bWaitDsp368ReadOutResponse=false;
                    waitDsp368ReadOutResponseCount=0;
                }
            }


            singleReadBtn->setDisabled(true);
            bSingleRead=false;
        }

    }
    else
    {
        qDebug() << "onStateChanged Unchecked";

        if(infoBtnGrup->checkedId()==4)
        {
            sht4xEnable = 0;

            QSettings settings("AppConfig.ini", QSettings::IniFormat);
            settings.setValue("SHT4x/SHT4x_Enable",sht4xEnable);

            //infoSHT4xComboBox[0]->setDisabled(false);
            //infoSHT4xComboBox[1]->setDisabled(false);
            singleReadBtn->setDisabled(false);
            //timer->stop();
            //ui->temperatureLabel->setText("Temperature:");
            //ui->humidtyLabel->setText("Humidty:");
            m_sht4xCard->clearValue1();
            m_sht4xCard->clearValue2();
        }
        else if(infoBtnGrup->checkedId()==5)
        {
            dsp368Enable = 0;

            QSettings settings("AppConfig.ini", QSettings::IniFormat);
            settings.setValue("DSP368/DSP368_Enable",dsp368Enable);


            singleReadBtn->setDisabled(false);

            //ui->pressureLabel->setText("Pressure:");
            //ui->tempDPS368Label->setText("Temperature:");
            m_dsp368Card->clearValue1();
            m_dsp368Card->clearValue2();
        }

    }
}

void MainWindow::getTwosComplement(int32_t *raw, uint8_t length)
{
    if (*raw & ((uint32_t)1 << (length - 1)))
    {
        *raw -= (uint32_t)1 << length;
    }
}

void MainWindow::initCoeff(uint8_t *buffer)
{
    qDebug() << "[MainWindow]initCoeff";

    //qDebug() <<"[MainWindow]initCoeff:";
    //for(int i=0;i<18;i++)
    //{
    //    qDebug()<<buffer[i]<<",";
    //}

    //compose coefficients from buffer content
    m_c0Half = ((uint32_t)buffer[0] << 4) | (((uint32_t)buffer[1] >> 4) & 0x0F);
    qDebug() <<"m_c0Half:"<<m_c0Half;
    getTwosComplement(&m_c0Half, 12);
    qDebug() <<"getTwosComplement m_c0Half:"<<m_c0Half;
    //c0 is only used as c0*0.5, so c0_half is calculated immediately
    m_c0Half = m_c0Half / 2U;

    //now do the same thing for all other coefficients
    m_c1 = (((uint32_t)buffer[1] & 0x0F) << 8) | (uint32_t)buffer[2];
    qDebug() <<"m_c1:"<<m_c1;
    getTwosComplement(&m_c1, 12);
    qDebug() <<"getTwosComplement m_c1:"<<m_c1;

    m_c00 = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | (((uint32_t)buffer[5] >> 4) & 0x0F);
    qDebug() <<"m_c00:"<<m_c00;
    getTwosComplement(&m_c00, 20);
    qDebug() <<"getTwosComplement m_c00:"<<m_c00;

    m_c10 = (((uint32_t)buffer[5] & 0x0F) << 16) | ((uint32_t)buffer[6] << 8) | (uint32_t)buffer[7];
    qDebug() <<"m_c10:"<<m_c10;
    getTwosComplement(&m_c10, 20);
    qDebug() <<"getTwosComplement m_c10:"<<m_c10;

    m_c01 = ((uint32_t)buffer[8] << 8) | (uint32_t)buffer[9];
    qDebug() <<"m_c01:"<<m_c01;
    getTwosComplement(&m_c01, 16);
    qDebug() <<"getTwosComplementm_c01:"<<m_c01;

    m_c11 = ((uint32_t)buffer[10] << 8) | (uint32_t)buffer[11];
    qDebug() <<"m_c11:"<<m_c11;
    getTwosComplement(&m_c11, 16);
    qDebug() <<"getTwosComplement m_c11:"<<m_c11;

    m_c20 = ((uint32_t)buffer[12] << 8) | (uint32_t)buffer[13];
    qDebug() <<"m_c20:"<<m_c20;
    getTwosComplement(&m_c20, 16);
    qDebug() <<"getTwosComplement m_c20:"<<m_c20;

    m_c21 = ((uint32_t)buffer[14] << 8) | (uint32_t)buffer[15];
    qDebug() <<"m_c21:"<<m_c21;
    getTwosComplement(&m_c21, 16);
    qDebug() <<"getTwosComplement m_c21:"<<m_c21;

    m_c30 = ((uint32_t)buffer[16] << 8) | (uint32_t)buffer[17];
    qDebug() <<"m_c30:"<<m_c30;
    getTwosComplement(&m_c30, 16);
    qDebug() <<"getTwosComplement m_c30:"<<m_c30;
}

float MainWindow::calcTemp(int32_t raw,int m_tempOsr)
{
    float temp = raw;

    //scale temperature according to scaling table and oversampling
    temp /= scaling_facts[m_tempOsr];

    //update last measured temperature
    //it will be used for pressure compensation
    m_lastTempScal = temp;

    //Calculate compensated temperature
    temp = m_c0Half + m_c1 * temp;

    return temp;
}

float MainWindow::calcPressure(int32_t raw,int m_prsOsr)
{
    float prs = raw;

    //scale pressure according to scaling table and oversampling
    prs /= scaling_facts[m_prsOsr];

    //Calculate compensated pressure
    prs = m_c00 + prs * (m_c10 + prs * (m_c20 + prs * m_c30)) + m_lastTempScal * (m_c01 + prs * (m_c11 + prs * m_c21));

    //return pressure
    return prs;
}

void MainWindow::readOutSHT4x()
{

    uint8_t CMD[5];

    if(!bUpgrade)
    {
        CMD[0]=getSHT4xCMD(sht4xCommandIndex);
        memcpy(&CMD[1],&heatingProtectionTemperature,4);

        GenCommand(GET_TH_CMD,CMD,5);
    }
}

void MainWindow::readOutDSP368()
{
     qDebug() << "[MainWindow]readOutDSP368";

    if(!isDSP368Exist)
    {
        qDebug() << "[MainWindow]DSP368 is not exist";
        if(checkDSP368RetryCount==0)
        {
            GenCommand(READ_DPS368_COEFF_CMD, NULL, 0);
            checkDSP368RetryCount++;
        }
        return;
    }

    uint8_t CMD[2];

    if(!bUpgrade)
    {
        CMD[0]=dsp368TempOSRIndex;
        CMD[1]=dsp368PressureOSRIndex;
        GenCommand(READ_DPS368_TEMP_AND_PRESSURE_CMD,CMD,2);
    }
}

void MainWindow::readSHT4xSerialNumberBtn_clicked()
{
    qDebug() << "[MainWindow]readSHT4xSerialNumberBtn_clicked";
    GenCommand(GET_SHT4X_SERIAL_NUM_CMD, NULL, 0);
}

void MainWindow::setSHT4xSoftrestBtn_clicked()
{
    qDebug() << "[MainWindow]setSHT4xSoftrestBtn_clicked";
    GenCommand(SHT4X_SOFT_RESET_CMD, NULL, 0);
}

void MainWindow::singleReadSHT4xBtn_clicked()
{
    qDebug() << "[MainWindow]singleReadSHT4xLogBtn_clicked";
    bSingleRead=true;
    readOutSHT4x();
}

void MainWindow::singleReadDSP368_clicked()
{
    qDebug() << "[MainWindow]singleReadDSP368_clicked";
    bSingleRead=true;
    readOutDSP368();
}

void MainWindow::sbm100_readFrom_clicked()
{
    qDebug() << "[MainWindow]sbm100_readFrom_clicked";
    uint8_t CMD[1];

    if(!bUpgrade)
    {
        CMD[0]=readFromIndex;
        GenCommand(READ_SBM100_BLOCK_CMD,CMD,1);
    }
}

void MainWindow::updateSBM100UI()
{
    if(dcb_bypass>1)
            dcb_bypass=0;

    if(infoSBM100ComboBox[0])
        infoSBM100ComboBox[0]->setCurrentIndex(dcb_bypass);

    if(dcb_cutoff>7)
            dcb_bypass=0;

    if(infoSBM100ComboBox[1])
        infoDPS368ComboBox[1]->setCurrentIndex(dcb_cutoff);

    if(infoSBM100LineEdit[0])
        infoSBM100LineEdit[0]->setText("0x"+QString("%1").arg(biquad_b0, 4, 16, QLatin1Char('0')).toUpper());

    if(infoSBM100LineEdit[1])
        infoSBM100LineEdit[1]->setText("0x"+QString("%1").arg(biquad_b1, 4, 16, QLatin1Char('0')).toUpper());

    if(infoSBM100LineEdit[2])
        infoSBM100LineEdit[2]->setText("0x"+QString("%1").arg(biquad_b2, 4, 16, QLatin1Char('0')).toUpper());

    if(infoSBM100LineEdit[3])
        infoSBM100LineEdit[3]->setText("0x"+QString("%1").arg(biquad_a1, 4, 16, QLatin1Char('0')).toUpper());

    if(infoSBM100LineEdit[4])
        infoSBM100LineEdit[4]->setText("0x"+QString("%1").arg(biquad_a2, 4, 16, QLatin1Char('0')).toUpper());

    if(infoSBM100ComboBox[2])
        infoSBM100ComboBox[2]->setCurrentIndex(biquad_coeff_write);

    if(infoSBM100ComboBox[3])
        infoSBM100ComboBox[3]->setCurrentIndex(biquad_bypass);

    if(infoSBM100ComboBox[4])
        infoSBM100ComboBox[4]->setCurrentIndex(out24not28);

    if(infoSBM100ComboBox[5])
        infoSBM100ComboBox[5]->setCurrentIndex(polarity);

    user_gain_index=get_user_gain_index(user_gain);

    if(infoSBM100slider)
    infoSBM100slider->setSliderPosition(user_gain_index);

}

void MainWindow::writeTo_clicked()
{
    qDebug() << "[MainWindow]writeTo_clicked";
    uint8_t CMD[19];

    if(!bUpgrade)
    {
        CMD[0]=writeToIndex;

        CMD[1]=0x00;
        CMD[2]=dcb_bypass<<4|dcb_cutoff;

        bool ok;
        biquad_b0 = static_cast<uint16_t>(infoSBM100LineEdit[0]->text().trimmed().toUInt(&ok,16));
        if(!ok)
        {
            QMessageBox::information(NULL, "MessageBox", "invalid biquad_b0!!");
        }
        biquad_b1 = static_cast<uint16_t>(infoSBM100LineEdit[1]->text().trimmed().toUInt(&ok,16));
        if(!ok)
        {
            QMessageBox::information(NULL, "MessageBox", "invalid biquad_b1!!");
        }
        biquad_b2 = static_cast<uint16_t>(infoSBM100LineEdit[2]->text().trimmed().toUInt(&ok,16));
        if(!ok)
        {
            QMessageBox::information(NULL, "MessageBox", "invalid biquad_b2!!");
        }
        biquad_a1 = static_cast<uint16_t>(infoSBM100LineEdit[3]->text().trimmed().toUInt(&ok,16));
        if(!ok)
        {
            QMessageBox::information(NULL, "MessageBox", "invalid biquad_a1!!");
        }
        biquad_a2 = static_cast<uint16_t>(infoSBM100LineEdit[4]->text().trimmed().toUInt(&ok,16));
        if(!ok)
        {
            QMessageBox::information(NULL, "MessageBox", "invalid biquad_a2!!");
        }

        CMD[3]=(biquad_b0&0xff00)>>8;
        CMD[4]=biquad_b0&0xff;

        CMD[5]=(biquad_b1&0xff00)>>8;
        CMD[6]=biquad_b1&0xff;

        CMD[7]=(biquad_b2&0xff00)>>8;
        CMD[8]=biquad_b2&0xff;

        CMD[9]=(biquad_a1&0xff00)>>8;
        CMD[10]=biquad_a1&0xff;

        CMD[11]=(biquad_a2&0xff00)>>8;
        CMD[12]=biquad_a2&0xff;

        CMD[13]=0x00;
        CMD[14]=biquad_coeff_write;

        CMD[15]=0x00;
        CMD[16]=biquad_bypass;

        CMD[17]=0x00;
        CMD[18]=out24not28<<6|polarity<<5|user_gain;

        GenCommand(WRITE_SBM100_BLOCK_CMD,CMD,19);
    }
}


void MainWindow::saveCalBtn_clicked()
{
    qDebug() << "[MainWindow]saveCalBtn_clicked";
    uint8_t CMD[8];

    tempCal= tempCalLineEdit->text().trimmed().toFloat();
    humidityCal= humidityCalLineEdit->text().trimmed().toFloat();

    qDebug() << "[MainWindow]tempCal:"<<tempCal;

    qDebug() << "[MainWindow]humidityCal:"<<humidityCal;

    memcpy(&CMD[0],&tempCal,sizeof(float));
    memcpy(&CMD[4],&humidityCal,sizeof(float));

    GenCommand(WRITE_SHT4X_CAL_CMD,CMD,8);
}

void MainWindow::readCalBtn_clicked()
{
    qDebug() << "[MainWindow]readCalBtn_clicked";

    GenCommand(READ_SHT4X_CAL_CMD,NULL,0);
}

void MainWindow::getTimeBtn_clicked()
{
    QDateTime now=QDateTime::currentDateTime();

    qDebug() << "[MainWindow]getTimeBtn_clicked time:"<<now;

    infoLineEdit[24]->setText(now.toString("yyyy-MM-dd HH:mm:ss"));
}

void MainWindow::createShowInfo()
{
    deletePage();

    qDebug() << "[MainWindow]createShowInfo";

    QFont monoFont("Consolas");

    for(int i=0;i<5;i++)
    {
        GroupBox[i] = new QGroupBox(this);
        GroupBox[i]->setStyleSheet(QStringLiteral("QGroupBox{border:1px solid white;font-weight: bold;font-size: 10pt;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top left;padding:0.3px;}"));
    }

    for(int i=0;i<31;i++)
    {
        descriptionLabel[i] = new QLabel(this);
        descriptionLabel[i]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        //QFont font = descriptionLabel[i]->font();
        //font.setPointSize(10);
        //font.setBold(true);
        monoFont.setStyleHint(QFont::Monospace);
        monoFont.setFixedPitch(true);
        monoFont.setPointSize(11);
        //monoFont.setBold(true);

        descriptionLabel[i]->setFont(monoFont);

    }

    infoNoteEdit = new QTextEdit(this);

    GroupBox[0]->setGeometry(450,100,400,220);
    GroupBox[0]->setTitle("Cartridge 1");
    GroupBox[0]->show();

    descriptionLabel[0]->setGeometry(460,120,385,20);
    descriptionLabel[0]->setText("Model Number:            ["+QString::fromLatin1(info.cartridge1ModelNumber)+"]");
    descriptionLabel[0]->show();

    descriptionLabel[1]->setGeometry(460,140,385,20);
    descriptionLabel[1]->setText("Version Letter:          ["+QString::fromLatin1(info.cartridge1VersionLetter)+"]");
    descriptionLabel[1]->show();

    descriptionLabel[2]->setGeometry(460,160,385,20);
    descriptionLabel[2]->setText("Serial Number:           ["+QString::fromLatin1(info.cartridge1SerialNumber)+"]");
    descriptionLabel[2]->show();

    descriptionLabel[3]->setGeometry(460,180,385,20);
    descriptionLabel[3]->setText("Sensitivity:             ["+QString::number(info.cartridge1Sensitivity,'f',2)+"]");
    descriptionLabel[3]->show();

    descriptionLabel[4]->setGeometry(460,200,385,20);
    descriptionLabel[4]->setText("Reference Frequency:     ["+QString::number(info.cartridge1ReferenceFrequency,'f',2)+"]");
    descriptionLabel[4]->show();

    descriptionLabel[5]->setGeometry(460,220,385,20);
    descriptionLabel[5]->setText("Units Code:              ["+QString::number(info.cartridge1UnitsCode)+"]");
    descriptionLabel[5]->show();

    descriptionLabel[6]->setGeometry(460,240,385,20);
    descriptionLabel[6]->setText("Frequency Range Min:     ["+QString::number(info.cartridge1FrequencyRangeMin)+"]");
    descriptionLabel[6]->show();

    descriptionLabel[7]->setGeometry(460,260,385,20);
    descriptionLabel[7]->setText("Frequency Range Max:     ["+QString::number(info.cartridge1FrequencyRangeMax)+"]");
    descriptionLabel[7]->show();

    descriptionLabel[8]->setGeometry(460,280,385,20);
    descriptionLabel[8]->setText("Channel Assignment:      ["+QString::number(info.cartridge1ChannelAssignment)+"]");
    descriptionLabel[8]->show();

    GroupBox[1]->setGeometry(860,100,400,220);
    GroupBox[1]->setTitle("Cartridge 2");
    GroupBox[1]->show();

    descriptionLabel[9]->setGeometry(870,120,385,20);
    descriptionLabel[9]->setText("Model Number:            ["+QString::fromLatin1(info.cartridge2ModelNumber)+"]");
    descriptionLabel[9]->show();

    descriptionLabel[10]->setGeometry(870,140,385,20);
    descriptionLabel[10]->setText("Version Letter:          ["+QString::fromLatin1(info.cartridge2VersionLetter)+"]");
    descriptionLabel[10]->show();

    descriptionLabel[11]->setGeometry(870,160,385,20);
    descriptionLabel[11]->setText("Serial Number:           ["+QString::fromLatin1(info.cartridge2SerialNumber)+"]");
    descriptionLabel[11]->show();

    descriptionLabel[12]->setGeometry(870,180,385,20);
    descriptionLabel[12]->setText("Sensitivity:             ["+QString::number(info.cartridge2Sensitivity,'f',2)+"]");
    descriptionLabel[12]->show();

    descriptionLabel[13]->setGeometry(870,200,385,20);
    descriptionLabel[13]->setText("Reference Frequency:     ["+QString::number(info.cartridge2ReferenceFrequency,'f',2)+"]");
    descriptionLabel[13]->show();

    descriptionLabel[14]->setGeometry(870,220,385,20);
    descriptionLabel[14]->setText("Units Code:              ["+QString::number(info.cartridge2UnitsCode)+"]");
    descriptionLabel[14]->show();

    descriptionLabel[15]->setGeometry(870,240,385,20);
    descriptionLabel[15]->setText("Frequency Range Min:     ["+QString::number(info.cartridge2FrequencyRangeMin)+"]");
    descriptionLabel[15]->show();

    descriptionLabel[16]->setGeometry(870,260,385,20);
    descriptionLabel[16]->setText("Frequency Range Max:     ["+QString::number(info.cartridge2FrequencyRangeMax)+"]");
    descriptionLabel[16]->show();

    descriptionLabel[17]->setGeometry(870,280,385,20);
    descriptionLabel[17]->setText("Channel Assignment:      ["+QString::number(info.cartridge2ChannelAssignment)+"]");
    descriptionLabel[17]->show();

    GroupBox[2]->setGeometry(450,330,400,210);
    GroupBox[2]->setTitle("System");
    GroupBox[2]->show();

    descriptionLabel[18]->setGeometry(460,350,385,20);
    descriptionLabel[18]->setText("Digital Interface Type:  ["+QString::fromLatin1(info.systemDigitInterfaceType)+"]");
    descriptionLabel[18]->show();

    descriptionLabel[19]->setGeometry(460,370,385,20);
    descriptionLabel[19]->setText("Bit Clock Frequency:     ["+QString::number(info.systemBitClockFrequency,'f',4)+"]");
    descriptionLabel[19]->show();

    descriptionLabel[20]->setGeometry(460,390,385,20);
    descriptionLabel[20]->setText("Word Length:             ["+QString::number(info.systemWordLength)+"]");
    descriptionLabel[20]->show();

    descriptionLabel[21]->setGeometry(460,410,385,20);
    descriptionLabel[21]->setText("Sample Rate:             ["+QString::number(info.systemSampleRate)+"]");
    descriptionLabel[21]->show();

    descriptionLabel[22]->setGeometry(460,430,385,20);
    descriptionLabel[22]->setText("Serial Number:           ["+QString::fromLatin1(info.systemSerialnumber)+"]");
    descriptionLabel[22]->show();

    descriptionLabel[23]->setGeometry(460,450,385,20);
    descriptionLabel[23]->setText("Sensitivity:             ["+QString::number(info.systemSensitivity,'f',2)+"]");
    descriptionLabel[23]->show();

    QDateTime cdate=bcdToDateTime(info.systemCalibrationDate);
    qDebug() << "Calibration Date:"<<cdate;

    descriptionLabel[24]->setGeometry(460,470,385,20);
    descriptionLabel[24]->setText("Calibration Date:        ["+cdate.toString("yyyy-MM-dd HH:mm:ss")+"]");
    descriptionLabel[24]->show();

    descriptionLabel[25]->setGeometry(460,490,385,20);
    descriptionLabel[25]->setText("Manufacturer ID:         [0x"+QString("%1").arg(info.systemManufactuerID, 4, 16, QLatin1Char('0')).toUpper()+"]");
    descriptionLabel[25]->show();
    descriptionLabel[26]->setGeometry(460,510,385,20);
    descriptionLabel[26]->setText("Firewarm Version:        [0x"+QString("%1").arg(version, 4, 16, QLatin1Char('0')).toUpper()+"]");
    descriptionLabel[26]->show();

    GroupBox[3]->setGeometry(860,330,400,210);
    GroupBox[3]->setTitle("Note");
    GroupBox[3]->show();

    QByteArray byteArray(reinterpret_cast<const char*>(note.note), 128);
    QString text = QString::fromUtf8(byteArray);
    text = text.left(text.indexOf(QChar('\0')));

    infoNoteEdit->setGeometry(870,350,380,180);
    infoNoteEdit->setStyleSheet("QTextEdit { background-color: #000000; }");
    infoNoteEdit->setFont(monoFont);
    infoNoteEdit->setReadOnly(true);
    infoNoteEdit->setPlainText(text);
    infoNoteEdit->show();

    GroupBox[4]->setGeometry(450,550,400,120);
    GroupBox[4]->setTitle("USB Hub Info");
    GroupBox[4]->show();

    descriptionLabel[27]->setGeometry(460,570,385,20);
    descriptionLabel[27]->setText("VID & PID:       [0x"+QString("%1").arg(mcpVID, 4, 16, QLatin1Char('0')).toUpper()+" & 0x"+QString("%1").arg(mcpPID, 4, 16, QLatin1Char('0')).toUpper()+"]");
    descriptionLabel[27]->show();

    descriptionLabel[28]->setGeometry(460,590,385,20);
    descriptionLabel[28]->setText("Descriptor:      ["+QString::fromWCharArray(ProdDescrip)+"]");
    descriptionLabel[28]->show();

    descriptionLabel[29]->setGeometry(460,610,385,20);
    descriptionLabel[29]->setText("Manufacturer:    ["+QString::fromWCharArray(MfrDescriptor)+"]");
    descriptionLabel[29]->show();

    descriptionLabel[30]->setGeometry(460,630,385,20);
    descriptionLabel[30]->setText("SerialNumber:    ["+QString::fromWCharArray(PordSerNum)+"]");
    descriptionLabel[30]->show();


}

void MainWindow::createEditNote()
{
    deletePage();

    qDebug() << "[MainWindow]createEditNote";

    descriptionLabel[0] = new QLabel(this);
    infoNoteEdit = new QTextEdit(this);
    writeConfig = new QPushButton(this);
    clearBtn = new QPushButton(this);

    descriptionLabel[0]->setGeometry(460,120,385,20);
    descriptionLabel[0]->setText("input Note(max length:128)");
    descriptionLabel[0]->show();

    QByteArray byteArray(reinterpret_cast<const char*>(note.note), 128);
    QString text = QString::fromUtf8(byteArray);
    text = text.left(text.indexOf(QChar('\0')));

    infoNoteEdit->setGeometry(460,145,380,180);
    infoNoteEdit->setStyleSheet("QTextEdit { background-color: #000000; }");
    infoNoteEdit->setPlainText(text);
    infoNoteEdit->show();

    oldValueList.clear();
    oldValueList.append(text);

    writeConfig->setGeometry(495,360,140,28);
    writeConfig->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }
    )");
    writeConfig->setText("Write Note");
    writeConfig->show();

    clearBtn->setGeometry(655,360,140,28);
    clearBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }
    )");
    clearBtn->setText("Clear");
    clearBtn->show();

    connect(writeConfig,SIGNAL(clicked()),this,SLOT(writeConfig_clicked()));
    connect(clearBtn,SIGNAL(clicked()),this,SLOT(clearBtn_clicked()));

    connect(infoNoteEdit, &QTextEdit::textChanged, this, [=]() {
        const int maxLen = 128;
        QString text = infoNoteEdit->toPlainText();
        if (text.length() > maxLen) {
            text = text.left(maxLen);
            infoNoteEdit->blockSignals(true);
            infoNoteEdit->setPlainText(text);
            infoNoteEdit->moveCursor(QTextCursor::End); // 保留游標位置
            infoNoteEdit->blockSignals(false);
        }
    });
}

void MainWindow::WriteInfoSuccessIncreaseSerialNumberOffset()
{
    SerialNumberOffset++;

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    settings.setValue("BURN_INFO/SerialNumberOffser",SerialNumberOffset);
}

void MainWindow::createEditInfo()
{
    deletePage();

    qDebug() << "[MainWindow]createEditInfo";

    for(int i=0;i<26;i++)
        descriptionLabel[i] = new QLabel(this);

    for(int i=0;i<26;i++)
    {
        infoLineEdit[i] = new QLineEdit(this);
        infoLineEdit[i]->setStyleSheet("QLineEdit { background-color: #000000; }");
    }

    getTimeBtn = new QPushButton(this);
    writeConfig = new QPushButton(this);

    QDoubleValidator* validator = new QDoubleValidator(this);
    validator->setNotation(QDoubleValidator::StandardNotation);  // 禁用科學記號（如1e10）
    validator->setDecimals(4);  // 最多允許 4 位小數
    validator->setRange(-999999, 999999);  // 設定可接受的範圍（也可不設）

    QIntValidator* intValidator = new QIntValidator(-32768, 32767, this);
    QIntValidator* byteValidator = new QIntValidator(0, 255, this);

    QRegularExpression regex(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");;
    QRegularExpressionValidator* dateValidator = new QRegularExpressionValidator(regex, this);

    QRegularExpression manufactuerIDregex("^0x[0-9A-Fa-f]{1,4}$");
    QRegularExpressionValidator* manufactuerIDvalidator = new QRegularExpressionValidator(manufactuerIDregex, this);

    descriptionLabel[0]->setGeometry(460,115,385,20);
    descriptionLabel[0]->setText("Cartridge 1 Model Number:");
    descriptionLabel[0]->show();

    infoLineEdit[0]->setGeometry(660,115,130,20);
    infoLineEdit[0]->setMaxLength(8);
    infoLineEdit[0]->setText(QString::fromLatin1(info.cartridge1ModelNumber));
    infoLineEdit[0]->show();

    descriptionLabel[1]->setGeometry(460,140,385,20);
    descriptionLabel[1]->setText("Cartridge 1 Version Letter:");
    descriptionLabel[1]->show();

    infoLineEdit[1]->setGeometry(660,140,130,20);
    infoLineEdit[1]->setMaxLength(1);
    infoLineEdit[1]->setText(QString::fromLatin1(info.cartridge1VersionLetter));
    infoLineEdit[1]->show();

    descriptionLabel[2]->setGeometry(460,165,385,20);
    descriptionLabel[2]->setText("Cartridge 1 Serial Number:");
    descriptionLabel[2]->show();

    infoLineEdit[2]->setGeometry(660,165,130,20);
    infoLineEdit[2]->setMaxLength(16);
    infoLineEdit[2]->setText(QString::fromLatin1(info.cartridge1SerialNumber));
    infoLineEdit[2]->show();

    descriptionLabel[3]->setGeometry(460,190,385,20);
    descriptionLabel[3]->setText("Cartridge 1 Sensitivity:");
    descriptionLabel[3]->show();

    infoLineEdit[3]->setGeometry(660,190,130,20);
    infoLineEdit[3]->setValidator(validator);
    infoLineEdit[3]->setText(QString::number(info.cartridge1Sensitivity,'f',2));
    infoLineEdit[3]->show();

    descriptionLabel[4]->setGeometry(460,215,385,20);
    descriptionLabel[4]->setText("Cartridge 1 Reference Frequency:");
    descriptionLabel[4]->show();

    infoLineEdit[4]->setGeometry(660,215,130,20);
    infoLineEdit[4]->setValidator(validator);
    infoLineEdit[4]->setText(QString::number(info.cartridge1ReferenceFrequency,'f',2));
    infoLineEdit[4]->show();

    descriptionLabel[5]->setGeometry(460,240,385,20);
    descriptionLabel[5]->setText("Cartridge 1 Units Code:");
    descriptionLabel[5]->show();

    infoLineEdit[5]->setGeometry(660,240,130,20);
    infoLineEdit[5]->setValidator(byteValidator);
    infoLineEdit[5]->setText(QString::number(info.cartridge1UnitsCode));
    infoLineEdit[5]->show();

    descriptionLabel[6]->setGeometry(460,265,385,20);
    descriptionLabel[6]->setText("Cartridge 1 Frequency Range Min:");
    descriptionLabel[6]->show();

    infoLineEdit[6]->setGeometry(660,265,130,20);
    infoLineEdit[6]->setValidator(intValidator);
    infoLineEdit[6]->setText(QString::number(info.cartridge1FrequencyRangeMin));
    infoLineEdit[6]->show();

    descriptionLabel[7]->setGeometry(460,290,385,20);
    descriptionLabel[7]->setText("Cartridge 1 Frequency Range Max:");
    descriptionLabel[7]->show();

    infoLineEdit[7]->setGeometry(660,290,130,20);
    infoLineEdit[7]->setValidator(intValidator);
    infoLineEdit[7]->setText(QString::number(info.cartridge1FrequencyRangeMax));
    infoLineEdit[7]->show();

    descriptionLabel[8]->setGeometry(460,315,385,20);
    descriptionLabel[8]->setText("Cartridge 1 Channel Assignment:");
    descriptionLabel[8]->show();

    infoLineEdit[8]->setGeometry(660,315,130,20);
    infoLineEdit[8]->setValidator(byteValidator);
    infoLineEdit[8]->setText(QString::number(info.cartridge1ChannelAssignment));
    infoLineEdit[8]->show();

    descriptionLabel[9]->setGeometry(800,115,385,20);
    descriptionLabel[9]->setText("Cartridge 2 Model Number:");
    descriptionLabel[9]->show();

    infoLineEdit[9]->setGeometry(1000,115,130,20);
    infoLineEdit[9]->setMaxLength(8);
    infoLineEdit[9]->setText(QString::fromLatin1(info.cartridge2ModelNumber));
    infoLineEdit[9]->show();

    descriptionLabel[10]->setGeometry(800,140,385,20);
    descriptionLabel[10]->setText("Cartridge 2 Version Letter:");
    descriptionLabel[10]->show();

    infoLineEdit[10]->setGeometry(1000,140,130,20);
    infoLineEdit[10]->setMaxLength(1);
    infoLineEdit[10]->setText(QString::fromLatin1(info.cartridge2VersionLetter));
    infoLineEdit[10]->show();

    descriptionLabel[11]->setGeometry(800,165,385,20);
    descriptionLabel[11]->setText("Cartridge 2 Serial Number:");
    descriptionLabel[11]->show();

    infoLineEdit[11]->setGeometry(1000,165,130,20);
    infoLineEdit[1]->setMaxLength(16);
    infoLineEdit[11]->setText(QString::fromLatin1(info.cartridge2SerialNumber));
    infoLineEdit[11]->show();

    descriptionLabel[12]->setGeometry(800,190,385,20);
    descriptionLabel[12]->setText("Cartridge 2 Sensitivity:");
    descriptionLabel[12]->show();

    infoLineEdit[12]->setGeometry(1000,190,130,20);
    infoLineEdit[12]->setValidator(validator);
    infoLineEdit[12]->setText(QString::number(info.cartridge2Sensitivity,'f',2));
    infoLineEdit[12]->show();

    descriptionLabel[13]->setGeometry(800,215,385,20);
    descriptionLabel[13]->setText("Cartridge 2 Reference Frequency:");
    descriptionLabel[13]->show();

    infoLineEdit[13]->setGeometry(1000,215,130,20);
    infoLineEdit[13]->setValidator(validator);
    infoLineEdit[13]->setText(QString::number(info.cartridge2ReferenceFrequency,'f',2));
    infoLineEdit[13]->show();

    descriptionLabel[14]->setGeometry(800,240,385,20);
    descriptionLabel[14]->setText("Cartridge 2 Units Code:");
    descriptionLabel[14]->show();

    infoLineEdit[14]->setGeometry(1000,240,130,20);
    infoLineEdit[14]->setValidator(byteValidator);
    infoLineEdit[14]->setText(QString::number(info.cartridge2UnitsCode));
    infoLineEdit[14]->show();

    descriptionLabel[15]->setGeometry(800,265,385,20);
    descriptionLabel[15]->setText("Cartridge 2 Frequency Range Min:");
    descriptionLabel[15]->show();

    infoLineEdit[15]->setGeometry(1000,265,130,20);
    infoLineEdit[15]->setValidator(intValidator);
    infoLineEdit[15]->setText(QString::number(info.cartridge2FrequencyRangeMin));
    infoLineEdit[15]->show();

    descriptionLabel[16]->setGeometry(800,290,385,20);
    descriptionLabel[16]->setText("Cartridge 2 Frequency Range Max:");
    descriptionLabel[16]->show();

    infoLineEdit[16]->setGeometry(1000,290,130,20);
    infoLineEdit[16]->setValidator(intValidator);
    infoLineEdit[16]->setText(QString::number(info.cartridge2FrequencyRangeMax));
    infoLineEdit[16]->show();

    descriptionLabel[17]->setGeometry(800,315,385,20);
    descriptionLabel[17]->setText("Cartridge 2 Channel Assignment:");
    descriptionLabel[17]->show();

    infoLineEdit[17]->setGeometry(1000,315,130,20);
    infoLineEdit[17]->setValidator(byteValidator);
    infoLineEdit[17]->setText(QString::number(info.cartridge2ChannelAssignment));
    infoLineEdit[17]->show();

    descriptionLabel[18]->setGeometry(460,365,385,20);
    descriptionLabel[18]->setText("System Digital Interface Type:");
    descriptionLabel[18]->show();

    infoLineEdit[18]->setGeometry(660,365,130,20);
    infoLineEdit[18]->setMaxLength(4);
    infoLineEdit[18]->setText(QString::fromLatin1(info.systemDigitInterfaceType));
    infoLineEdit[18]->show();

    descriptionLabel[19]->setGeometry(460,390,385,20);
    descriptionLabel[19]->setText("System Bit Clock Frequency:");
    descriptionLabel[19]->show();

    infoLineEdit[19]->setGeometry(660,390,130,20);
    infoLineEdit[19]->setValidator(validator);
    infoLineEdit[19]->setText(QString::number(info.systemBitClockFrequency,'f',4));
    infoLineEdit[19]->show();

    descriptionLabel[20]->setGeometry(460,415,385,20);
    descriptionLabel[20]->setText("System Word Length:");
    descriptionLabel[20]->show();

    infoLineEdit[20]->setGeometry(660,415,130,20);
    infoLineEdit[20]->setValidator(byteValidator);
    infoLineEdit[20]->setText(QString::number(info.systemWordLength));
    infoLineEdit[20]->show();

    descriptionLabel[21]->setGeometry(460,440,385,20);
    descriptionLabel[21]->setText("System Sample Rate:");
    descriptionLabel[21]->show();

    infoLineEdit[21]->setGeometry(660,440,130,20);
    infoLineEdit[21]->setValidator(validator);
    infoLineEdit[21]->setText(QString::number(info.systemSampleRate,'f',3));
    infoLineEdit[21]->show();

    descriptionLabel[22]->setGeometry(460,465,385,20);
    descriptionLabel[22]->setText("System Serial Number:");
    descriptionLabel[22]->show();

    infoLineEdit[22]->setGeometry(660,465,130,20);
    infoLineEdit[22]->setMaxLength(16);
    infoLineEdit[22]->setText(QString::fromLatin1(info.systemSerialnumber));
    infoLineEdit[22]->show();

    descriptionLabel[23]->setGeometry(460,490,385,20);
    descriptionLabel[23]->setText("System Sensitivity:");
    descriptionLabel[23]->show();

    infoLineEdit[23]->setGeometry(660,490,130,20);
    infoLineEdit[23]->setValidator(validator);
    infoLineEdit[23]->setText(QString::number(info.systemSensitivity,'f',2));
    infoLineEdit[23]->show();

    descriptionLabel[24]->setGeometry(460,515,385,20);
    descriptionLabel[24]->setText("System Calibration Date:");
    descriptionLabel[24]->show();

    QDateTime cdate=bcdToDateTime(info.systemCalibrationDate);
    qDebug() << "Calibration Date:"<<cdate;

    infoLineEdit[24]->setGeometry(660,515,160,20);
    infoLineEdit[24]->setPlaceholderText("YYYY-MM-DD HH:mm:ss");
    infoLineEdit[24]->setValidator(dateValidator);
    infoLineEdit[24]->setText(cdate.toString("yyyy-MM-dd HH:mm:ss"));
    infoLineEdit[24]->show();

    getTimeBtn->setGeometry(825,515,80,20);
    getTimeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }
    )");
    getTimeBtn->setText("Get Time");
    getTimeBtn->show();
    connect(getTimeBtn,SIGNAL(clicked()),this,SLOT(getTimeBtn_clicked()));

    descriptionLabel[25]->setGeometry(460,540,385,20);
    descriptionLabel[25]->setText("System Manufacturer ID:");
    descriptionLabel[25]->show();

    infoLineEdit[25]->setGeometry(660,540,130,20);
    infoLineEdit[25]->setValidator(manufactuerIDvalidator);
    infoLineEdit[25]->setPlaceholderText("0x0000");
    infoLineEdit[25]->setText("0x"+QString("%1").arg(info.systemManufactuerID, 4, 16, QLatin1Char('0')).toUpper());
    infoLineEdit[25]->show();

    if(burnInfo)
    {
        infoLineEdit[0]->setText(C1ModelNumber);
        infoLineEdit[1]->setText(C1VersionLetter);
        infoLineEdit[2]->setText(C1SerialNumber+QString("%1").arg(SerialNumberOffset, 5, 10, QLatin1Char('0')));
        infoLineEdit[3]->setText(C1Sensitivity);
        infoLineEdit[4]->setText(C1ReferenceFrequency);
        infoLineEdit[5]->setText(C1UnitsCode);
        infoLineEdit[6]->setText(C1FrequencyRangeMin);
        infoLineEdit[7]->setText(C1FrequencyRangeMax);
        infoLineEdit[8]->setText(C1ChannelAssignment);
        infoLineEdit[9]->setText(C2ModelNumber);
        infoLineEdit[10]->setText(C2VersionLetter);
        infoLineEdit[11]->setText(C2SerialNumber+QString("%1").arg(SerialNumberOffset, 5, 10, QLatin1Char('0')));
        infoLineEdit[12]->setText(C2Sensitivity);
        infoLineEdit[13]->setText(C2ReferenceFrequency);
        infoLineEdit[14]->setText(C2UnitsCode);
        infoLineEdit[15]->setText(C2FrequencyRangeMin);
        infoLineEdit[16]->setText(C2FrequencyRangeMax);
        infoLineEdit[17]->setText(C2ChannelAssignment);
        infoLineEdit[18]->setText(systemDigitInterfaceType);
        infoLineEdit[19]->setText(systemBitClockFrequency);
        infoLineEdit[20]->setText(systemWordLength);
        infoLineEdit[21]->setText(systemSampleRate);
        infoLineEdit[22]->setText(systemSerialnumber+QString("%1").arg(SerialNumberOffset, 4, 10, QLatin1Char('0')));
        infoLineEdit[23]->setText(systemSensitivity);
        infoLineEdit[24]->setText(systemCalibrationDate);
        infoLineEdit[25]->setText(systemManufactuerID);
    }

    writeConfig->setGeometry(460,590,140,28);
    writeConfig->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }
    )");
    oldValueList.clear();
    for(int i=0;i<26;i++)
    {
        oldValueList.append(infoLineEdit[i]->text());
    }

    writeConfig->setText("Write Info");
    writeConfig->show();
    connect(writeConfig,SIGNAL(clicked()),this,SLOT(writeConfig_clicked()));
}

void MainWindow::createEditHubInfo()
{
    deletePage();

    qDebug() << "[MainWindow]createEditHubInfo";

    for(int i=0;i<5;i++)
        descriptionLabel[i] = new QLabel(this);

    for(int i=0;i<5;i++)
    {
        infoLineEdit[i] = new QLineEdit(this);
        infoLineEdit[i]->setStyleSheet("QLineEdit { background-color: #000000; }");
    }

    writeConfig = new QPushButton(this);

    QRegularExpression manufactuerIDregex("^0x[0-9A-Fa-f]{1,4}$");
    QRegularExpressionValidator* manufactuerIDvalidator = new QRegularExpressionValidator(manufactuerIDregex, this);

    descriptionLabel[0]->setGeometry(460,115,385,20);
    descriptionLabel[0]->setText("VID:");
    descriptionLabel[0]->show();

    infoLineEdit[0]->setGeometry(580,115,240,20);
    infoLineEdit[0]->setValidator(manufactuerIDvalidator);
    infoLineEdit[0]->setPlaceholderText("0x0000");
    infoLineEdit[0]->setText("0x"+QString("%1").arg(mcpVID, 4, 16, QLatin1Char('0')).toUpper());
    infoLineEdit[0]->show();

    descriptionLabel[1]->setGeometry(460,140,385,20);
    descriptionLabel[1]->setText("PID:");
    descriptionLabel[1]->show();

    infoLineEdit[1]->setGeometry(580,140,240,20);
    infoLineEdit[1]->setValidator(manufactuerIDvalidator);
    infoLineEdit[1]->setPlaceholderText("0x0000");
    infoLineEdit[1]->setText("0x"+QString("%1").arg(mcpPID, 4, 16, QLatin1Char('0')).toUpper());
    infoLineEdit[1]->show();


    descriptionLabel[2]->setGeometry(460,165,385,20);
    descriptionLabel[2]->setText("Descriptor:");
    descriptionLabel[2]->show();

    infoLineEdit[2]->setGeometry(580,165,240,20);
    infoLineEdit[2]->setMaxLength(30);
    infoLineEdit[2]->setText(QString::fromWCharArray(ProdDescrip));
    infoLineEdit[2]->show();

    descriptionLabel[3]->setGeometry(460,190,385,20);
    descriptionLabel[3]->setText("Manufacturer:");
    descriptionLabel[3]->show();

    infoLineEdit[3]->setGeometry(580,190,240,20);
    infoLineEdit[3]->setMaxLength(30);
    infoLineEdit[3]->setText(QString::fromWCharArray(MfrDescriptor));
    infoLineEdit[3]->show();

    descriptionLabel[4]->setGeometry(460,215,385,20);
    descriptionLabel[4]->setText("SerialNumber:");
    descriptionLabel[4]->show();

    infoLineEdit[4]->setGeometry(580,215,240,20);
    infoLineEdit[4]->setMaxLength(30);
    infoLineEdit[4]->setText(QString::fromWCharArray(PordSerNum));
    infoLineEdit[4]->show();

    oldValueList.clear();
    for(int i=0;i<5;i++)
    {
        oldValueList.append(infoLineEdit[i]->text());
    }

    writeConfig->setGeometry(460,265,140,28);
    writeConfig->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }
    )");
    writeConfig->setText("Write Hub Info");
    writeConfig->show();

    connect(writeConfig,SIGNAL(clicked()),this,SLOT(writeConfig_clicked()));
}

int MainWindow::get_user_gain_index(uint8_t reg)
{
    int8_t val = reg;

    if(val&0x10)
        val|=0xE0;

    return val;
}

void MainWindow::createSBM100Setting()
{
    qDebug() << "[MainWindow]createDPS368Setting";


    deletePage();

    for(int i=0;i<10;i++)
    {
        GroupBox[i] = new QGroupBox(this);
        GroupBox[i]->setStyleSheet(QStringLiteral("QGroupBox{border:1px solid white;font-weight: bold;font-size: 10pt;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top left;padding:0.3px;}"));
    }

    for(int i=0;i<20;i++)
        descriptionLabel[i] = new QLabel(this);

    for(int i=0;i<5;i++)
    {
        infoSBM100LineEdit[i] = new QLineEdit(this);
        infoSBM100LineEdit[i]->setStyleSheet("QLineEdit { background-color: #000000; }");
    }

    GroupBox[0]->setGeometry(450,100,320,140);
    GroupBox[0]->setTitle("REG:0x120 DC Blocker Config");
    GroupBox[0]->show();

    descriptionLabel[0]->setGeometry(460,120,120,20);
    descriptionLabel[0]->setText("dcb_bypass:");
    descriptionLabel[0]->show();

    infoSBM100ComboBox[0] = new QComboBox(this);
    infoSBM100ComboBox[0]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoSBM100ComboBox[0]->addItem("0 - Disabled");
    infoSBM100ComboBox[0]->addItem("1 - Enabled");
    if(dcb_bypass>1)
        dcb_bypass=0;
    infoSBM100ComboBox[0]->setCurrentIndex(dcb_bypass);
    infoSBM100ComboBox[0]->setGeometry(460,140,130,20);

    infoSBM100ComboBox[0]->show();

    connect(infoSBM100ComboBox[0],SIGNAL(currentIndexChanged(int)),this,SLOT(SMB100_dcb_bypass_SlelectChange(int)));

    descriptionLabel[1]->setGeometry(460,165,240,40);
    descriptionLabel[1]->setText("dcb_cutoff:\r\nHigh pass filter ASIC cut-off frequency");
    descriptionLabel[1]->show();

    infoSBM100ComboBox[1] = new QComboBox(this);
    infoSBM100ComboBox[1]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoSBM100ComboBox[1]->addItem("000: 0.93Hz");
    infoSBM100ComboBox[1]->addItem("001: 1.87Hz");
    infoSBM100ComboBox[1]->addItem("010: 3.74Hz");
    infoSBM100ComboBox[1]->addItem("011: 7.48Hz");
    infoSBM100ComboBox[1]->addItem("100: 15.0Hz");
    infoSBM100ComboBox[1]->addItem("101: 29.9Hz");
    infoSBM100ComboBox[1]->addItem("110: 59.8Hz");
    infoSBM100ComboBox[1]->addItem("111: 119.7Hz");

    if(dcb_cutoff>7)
        dcb_bypass=0;
    infoSBM100ComboBox[1]->setCurrentIndex(dcb_cutoff);
    infoSBM100ComboBox[1]->setGeometry(460,205,130,20);

    infoSBM100ComboBox[1]->show();

    connect(infoSBM100ComboBox[1],SIGNAL(currentIndexChanged(int)),this,SLOT(SMB100_dcb_cutoff_SlelectChange(int)));

    GroupBox[1]->setGeometry(450,250,320,80);
    GroupBox[1]->setTitle("REG:0x130 biquad_b0");
    GroupBox[1]->show();

    descriptionLabel[2]->setGeometry(460,270,240,20);
    descriptionLabel[2]->setText("b0 coefficient of biquad filter");
    descriptionLabel[2]->show();

    QRegularExpression biquadregex("^0x[0-9A-Fa-f]{1,4}$");
    QRegularExpressionValidator* biquadvalidator = new QRegularExpressionValidator(biquadregex, this);

    infoSBM100LineEdit[0]->setGeometry(460,290,130,20);
    infoSBM100LineEdit[0]->setValidator(biquadvalidator);
    infoSBM100LineEdit[0]->setPlaceholderText("0x0000");
    infoSBM100LineEdit[0]->setText("0x"+QString("%1").arg(biquad_b0, 4, 16, QLatin1Char('0')).toUpper());
    //infoLineEdit[25]->setText("0x0000");
    infoSBM100LineEdit[0]->show();

    GroupBox[2]->setGeometry(450,340,320,80);
    GroupBox[2]->setTitle("REG:0x131 biquad_b1");
    GroupBox[2]->show();

    descriptionLabel[3]->setGeometry(460,360,240,20);
    descriptionLabel[3]->setText("b1 coefficient of biquad filter");
    descriptionLabel[3]->show();

    infoSBM100LineEdit[1]->setGeometry(460,380,130,20);
    infoSBM100LineEdit[1]->setValidator(biquadvalidator);
    infoSBM100LineEdit[1]->setPlaceholderText("0x0000");
    infoSBM100LineEdit[1]->setText("0x"+QString("%1").arg(biquad_b1, 4, 16, QLatin1Char('0')).toUpper());
    //infoLineEdit[25]->setText("0x0000");
    infoSBM100LineEdit[1]->show();

    GroupBox[3]->setGeometry(450,430,320,80);
    GroupBox[3]->setTitle("REG:0x132 biquad_b2");
    GroupBox[3]->show();

    descriptionLabel[4]->setGeometry(460,450,240,20);
    descriptionLabel[4]->setText("b2 coefficient of biquad filter");
    descriptionLabel[4]->show();

    infoSBM100LineEdit[2]->setGeometry(460,470,130,20);
    infoSBM100LineEdit[2]->setValidator(biquadvalidator);
    infoSBM100LineEdit[2]->setPlaceholderText("0x0000");
    infoSBM100LineEdit[2]->setText("0x"+QString("%1").arg(biquad_b2, 4, 16, QLatin1Char('0')).toUpper());
    //infoLineEdit[25]->setText("0x0000");
    infoSBM100LineEdit[2]->show();

    GroupBox[4]->setGeometry(450,520,320,80);
    GroupBox[4]->setTitle("REG:0x133 biquad_a1");
    GroupBox[4]->show();

    descriptionLabel[5]->setGeometry(460,540,240,20);
    descriptionLabel[5]->setText("a1 coefficient of biquad filter");
    descriptionLabel[5]->show();

    infoSBM100LineEdit[3]->setGeometry(460,560,130,20);
    infoSBM100LineEdit[3]->setValidator(biquadvalidator);
    infoSBM100LineEdit[3]->setPlaceholderText("0x0000");
    infoSBM100LineEdit[3]->setText("0x"+QString("%1").arg(biquad_a1, 4, 16, QLatin1Char('0')).toUpper());
    //infoLineEdit[25]->setText("0x0000");
    infoSBM100LineEdit[3]->show();

    GroupBox[5]->setGeometry(800,100,320,80);
    GroupBox[5]->setTitle("REG:0x134 biquad_b2");
    GroupBox[5]->show();

    descriptionLabel[6]->setGeometry(810,120,240,20);
    descriptionLabel[6]->setText("a2 coefficient of biquad filter");
    descriptionLabel[6]->show();

    infoSBM100LineEdit[4]->setGeometry(810,140,130,20);
    infoSBM100LineEdit[4]->setValidator(biquadvalidator);
    infoSBM100LineEdit[4]->setPlaceholderText("0x0000");
    infoSBM100LineEdit[4]->setText("0x"+QString("%1").arg(biquad_a2, 4, 16, QLatin1Char('0')).toUpper());
    //infoLineEdit[25]->setText("0x0000");
    infoSBM100LineEdit[4]->show();

    GroupBox[6]->setGeometry(800,190,320,80);
    GroupBox[6]->setTitle("REG:0x135 biquad_coeff_write");
    GroupBox[6]->show();

    descriptionLabel[7]->setGeometry(810,210,240,20);
    descriptionLabel[7]->setText("Biquad Coefficient Write");
    descriptionLabel[7]->show();

    infoSBM100ComboBox[2] = new QComboBox(this);
    infoSBM100ComboBox[2]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoSBM100ComboBox[2]->addItem("0 - Neglect");
    infoSBM100ComboBox[2]->addItem("1 - apply");
    infoSBM100ComboBox[2]->setCurrentIndex(biquad_coeff_write);
    infoSBM100ComboBox[2]->setGeometry(810,235,130,20);

    infoSBM100ComboBox[2]->show();

    connect(infoSBM100ComboBox[2],SIGNAL(currentIndexChanged(int)),this,SLOT(SMB100_biquad_coeff_write_SlelectChange(int)));

    GroupBox[7]->setGeometry(800,280,320,80);
    GroupBox[7]->setTitle("REG:0x136 biquad_bypass");
    GroupBox[7]->show();

    descriptionLabel[8]->setGeometry(810,300,240,20);
    descriptionLabel[8]->setText("Biquad filter bypass");
    descriptionLabel[8]->show();

    infoSBM100ComboBox[3] = new QComboBox(this);
    infoSBM100ComboBox[3]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoSBM100ComboBox[3]->addItem("0 - Disabled");
    infoSBM100ComboBox[3]->addItem("1 - Enabled");
    infoSBM100ComboBox[3]->setCurrentIndex(biquad_bypass);
    infoSBM100ComboBox[3]->setGeometry(810,325,130,20);

    infoSBM100ComboBox[3]->show();

    connect(infoSBM100ComboBox[3],SIGNAL(currentIndexChanged(int)),this,SLOT(SMB100_biquad_bypass_SlelectChange(int)));

    GroupBox[8]->setGeometry(800,370,320,225);
    GroupBox[8]->setTitle("REG:0x180 Gain, polarity, bit depth");
    GroupBox[8]->show();

    descriptionLabel[9]->setGeometry(810,390,240,20);
    descriptionLabel[9]->setText("out24not28 ");
    descriptionLabel[9]->show();

    infoSBM100ComboBox[4] = new QComboBox(this);
    infoSBM100ComboBox[4]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoSBM100ComboBox[4]->addItem("0: 28-bit output");
    infoSBM100ComboBox[4]->addItem("1: 24-bit output");
    infoSBM100ComboBox[4]->setCurrentIndex(out24not28);
    infoSBM100ComboBox[4]->setGeometry(810,415,130,20);

    infoSBM100ComboBox[4]->show();

    connect(infoSBM100ComboBox[4],SIGNAL(currentIndexChanged(int)),this,SLOT(SMB100_out24not28_SlelectChange(int)));

    descriptionLabel[10]->setGeometry(810,445,290,30);
    descriptionLabel[10]->setText("polarity:\r\nSet polarity of signal vs incoming acoustic pressure.");
    descriptionLabel[10]->show();

    infoSBM100ComboBox[5] = new QComboBox(this);
    infoSBM100ComboBox[5]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoSBM100ComboBox[5]->addItem("0: non-inverting");
    infoSBM100ComboBox[5]->addItem("1: inverting");
    infoSBM100ComboBox[5]->setCurrentIndex(polarity);
    infoSBM100ComboBox[5]->setGeometry(810,485,130,20);

    infoSBM100ComboBox[5]->show();

    connect(infoSBM100ComboBox[5],SIGNAL(currentIndexChanged(int)),this,SLOT(SMB100_polarity_SlelectChange(int)));

    user_gain_index=get_user_gain_index(user_gain);
    descriptionLabel[11]->setGeometry(810,515,290,20);
    descriptionLabel[11]->setText("user_gain: "+QString("%1 dB (REG:0x%2)").arg(user_gain_index * 3).arg(user_gain,2,16,QChar('0')));
    descriptionLabel[11]->show();


    infoSBM100slider = new QSlider(Qt::Horizontal,this);
    infoSBM100slider->setRange(-10,10);

    infoSBM100slider->setGeometry(810,540,300,30);
    infoSBM100slider->setSliderPosition(user_gain_index);
    infoSBM100slider->setTickInterval(1);
    infoSBM100slider->setTickPosition(QSlider::TicksBelow);
    infoSBM100slider->show();

    connect(infoSBM100slider,SIGNAL(valueChanged(int)),this,SLOT(SMB100_user_gain_index_SlelectChange(int)));


    readFromBtn = new QPushButton(this);

    readFromBtn->setGeometry(450,620,70,20);
    readFromBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }

        QPushButton:disabled {
        background-color: #E5ECF6;
        color: #8A8A8A;

        }
    )");
    readFromBtn->setText("Refresh");
    readFromBtn->show();
    connect(readFromBtn,SIGNAL(clicked()),this,SLOT(sbm100_readFrom_clicked()));

    if(sbm100_breadFrom)
    {
    infoSBM100ComboBox[6] = new QComboBox(this);
    infoSBM100ComboBox[6]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoSBM100ComboBox[6]->addItem("from SMB100");
    infoSBM100ComboBox[6]->addItem("from EEPROM");
    infoSBM100ComboBox[6]->setCurrentIndex(readFromIndex);
    infoSBM100ComboBox[6]->setGeometry(520,620,130,20);

    infoSBM100ComboBox[6]->show();

    connect(infoSBM100ComboBox[6],SIGNAL(currentIndexChanged(int)),this,SLOT(SMB100_readFrom_SlelectChange(int)));
    }
    writeToBtn = new QPushButton(this);

    writeToBtn->setGeometry(660,620,70,20);
    writeToBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }

        QPushButton:disabled {
        background-color: #E5ECF6;
        color: #8A8A8A;

        }
    )");
    writeToBtn->setText("Save All");
    writeToBtn->show();
    connect(writeToBtn,SIGNAL(clicked()),this,SLOT(writeTo_clicked()));
    if(sbm100_bwriteTo)
    {
    infoSBM100ComboBox[7] = new QComboBox(this);
    infoSBM100ComboBox[7]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoSBM100ComboBox[7]->addItem("to SMB100");
    infoSBM100ComboBox[7]->addItem("to EEPROM");
    infoSBM100ComboBox[7]->setCurrentIndex(writeToIndex);
    infoSBM100ComboBox[7]->setGeometry(730,620,130,20);

    infoSBM100ComboBox[7]->show();

    connect(infoSBM100ComboBox[7],SIGNAL(currentIndexChanged(int)),this,SLOT(SMB100_writeTo_SlelectChange(int)));
    }



}

void MainWindow::createDPS368Setting()
{
    qDebug() << "[MainWindow]createDPS368Setting";
    int offsetIndex=1;

    deletePage();

    for(int i=0;i<6;i++)
    {
        GroupBox[i] = new QGroupBox(this);
        GroupBox[i]->setStyleSheet(QStringLiteral("QGroupBox{border:1px solid white;font-weight: bold;font-size: 10pt;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top left;padding:0.3px;}"));
    }

    for(int i=0;i<6;i++)
        descriptionLabel[i] = new QLabel(this);

    GroupBox[0]->setGeometry(450,100,320,190);
    GroupBox[0]->setTitle("param setting");
    GroupBox[0]->show();

    descriptionLabel[0]->setGeometry(460,135,120,20);
    descriptionLabel[0]->setText("Temp Oversample:");
    descriptionLabel[0]->show();

    infoDPS368ComboBox[0] = new QComboBox(this);
    infoDPS368ComboBox[0]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoDPS368ComboBox[0]->addItem("0000 - single");
    infoDPS368ComboBox[0]->addItem("0001 - 2 times");
    infoDPS368ComboBox[0]->addItem("0010 - 4 times");
    infoDPS368ComboBox[0]->addItem("0011 - 8 times");
    infoDPS368ComboBox[0]->addItem("0100 - 16 times");
    infoDPS368ComboBox[0]->addItem("0101 - 32 times");
    infoDPS368ComboBox[0]->addItem("0110 - 64 times");
    infoDPS368ComboBox[0]->addItem("0111 - 128 times");

    infoDPS368ComboBox[0]->setCurrentIndex(dsp368TempOSRIndex);
    infoDPS368ComboBox[0]->setGeometry(590,135,130,20);

    infoDPS368ComboBox[0]->show();

    connect(infoDPS368ComboBox[0],SIGNAL(currentIndexChanged(int)),this,SLOT(DPS368TempOversampleSlelectChange(int)));

    descriptionLabel[1]->setGeometry(460,165,120,20);
    descriptionLabel[1]->setText("Pressure Oversample:");
    descriptionLabel[1]->show();

    infoDPS368ComboBox[1] = new QComboBox(this);
    infoDPS368ComboBox[1]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoDPS368ComboBox[1]->addItem("0000 - single");
    infoDPS368ComboBox[1]->addItem("0001 - 2 times");
    infoDPS368ComboBox[1]->addItem("0010 - 4 times");
    infoDPS368ComboBox[1]->addItem("0011 - 8 times");
    infoDPS368ComboBox[1]->addItem("0100 - 16 times");
    infoDPS368ComboBox[1]->addItem("0101 - 32 times");
    infoDPS368ComboBox[1]->addItem("0110 - 64 times");
    infoDPS368ComboBox[1]->addItem("0111 - 128 times");

    infoDPS368ComboBox[1]->setCurrentIndex(dsp368PressureOSRIndex);
    infoDPS368ComboBox[1]->setGeometry(590,165,130,20);

    infoDPS368ComboBox[1]->show();

    connect(infoDPS368ComboBox[1],SIGNAL(currentIndexChanged(int)),this,SLOT(DPS368PressureOversampleSlelectChange(int)));


    descriptionLabel[2]->setGeometry(460,195,120,20);
    descriptionLabel[2]->setText("Time Interval:");
    descriptionLabel[2]->show();

    infoDPS368ComboBox[2] = new QComboBox(this);
    infoDPS368ComboBox[2]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoDPS368ComboBox[2]->addItem("250ms");
    infoDPS368ComboBox[2]->addItem("500ms");
    for(int i=offsetIndex;i<61;i++)
    {
        infoDPS368ComboBox[2]->addItem(QLocale().toString(i)+"s");
    }

    infoDPS368ComboBox[2]->setGeometry(590,195,80,20);
    infoDPS368ComboBox[2]->setCurrentIndex(dsp368ReadoutIntervalIndex);

    infoDPS368ComboBox[2]->show();

    connect(infoDPS368ComboBox[2],SIGNAL(currentIndexChanged(int)),this,SLOT(DPS368IntervalSlelectChange(int)));

    singleReadBtn = new QPushButton(this);

    singleReadBtn->setGeometry(585,235,88,27);
    singleReadBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }

        QPushButton:disabled {
        background-color: #E5ECF6;
        color: #8A8A8A;

        }
    )");
    singleReadBtn->setText("Single read");
    singleReadBtn->show();
    connect(singleReadBtn,SIGNAL(clicked()),this,SLOT(singleReadDSP368_clicked()));


    startCheckBox = new QCheckBox("Start",this);
    startCheckBox->setGeometry(460,230,60,20);
    startCheckBox->setStyleSheet(
        "QCheckBox::indicator {"
        "width:13px;"
        "height:13px;"
        "border:1px solid #888;"
        "border-radius:3px;"
        "background:white;"
        "}"

        "QCheckBox::indicator:checked {"
        "background:#007AFF;"
        "border:1px solid #007AFF;"
        "}"

        "QCheckBox::indicator:hover {"
        "border:2px solid #007AFF;"
        "}"

        );



    startCheckBox->show();

    connect(startCheckBox, &QCheckBox::checkStateChanged,this, &MainWindow::onStateChanged);

    if(dsp368Enable)
    {
        startCheckBox->setChecked(true);
    }
    else
    {
        startCheckBox->setChecked(false);
    }

    enableLogCheckBox= new QCheckBox("Enable log",this);
    enableLogCheckBox->setGeometry(460,250,82,20);
    enableLogCheckBox->setStyleSheet(
        "QCheckBox::indicator {"
        "width:13px;"
        "height:13px;"
        "border:1px solid #888;"
        "border-radius:3px;"
        "background:white;"
        "}"

        "QCheckBox::indicator:checked {"
        "background:#007AFF;"
        "border:1px solid #007AFF;"
        "}"

        "QCheckBox::indicator:hover {"
        "border:2px solid #007AFF;"
        "}"

        );



    enableLogCheckBox->show();

    connect(enableLogCheckBox, &QCheckBox::checkStateChanged,this, &MainWindow::onLogStateChanged);

    if(dsp368EnableLog)
    {
        enableLogCheckBox->setChecked(true);
    }
    else
    {
        enableLogCheckBox->setChecked(false);
    }


}

void MainWindow::createSHT4xSetting()
{
    qDebug() << "[MainWindow]createSHT4xSetting";

    deletePage();

    int offsetIndex=1;

    QDoubleValidator* validator = new QDoubleValidator(this);
    validator->setNotation(QDoubleValidator::StandardNotation);  // 禁用科學記號（如1e10）
    validator->setDecimals(2);  // 最多允許 4 位小數
    validator->setRange(-999999, 999999);  // 設定可接受的範圍（也可不設）


    //QSettings settings("AppConfig.ini", QSettings::IniFormat);
    //sht4xCommandIndex=settings.value("SHT4x/CommandType", "0").toInt();
    for(int i=0;i<6;i++)
    {
        GroupBox[i] = new QGroupBox(this);
        GroupBox[i]->setStyleSheet(QStringLiteral("QGroupBox{border:1px solid white;font-weight: bold;font-size: 10pt;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top left;padding:0.3px;}"));
    }

    for(int i=0;i<6;i++)
        descriptionLabel[i] = new QLabel(this);

   baseTemp= new QLabel(this);
   baseHumidity= new QLabel(this);
   heatingTemp= new QLabel(this);
   heatingHumidity= new QLabel(this);


    GroupBox[4]->setGeometry(450,100,320,140);
    GroupBox[4]->setTitle("readout");
    GroupBox[4]->show();

    baseTemp->setGeometry(460,145,140,20);
    if(sht4xCommandIndex>2)
        baseTemp->setText("Base Temp:       "+QString::number((notifyTemp/1000.0), 'f', 3));
    else
        baseTemp->setText("Base Temp:       "+QString::number((temp/1000.0), 'f', 3));

    baseTemp->show();


    baseHumidity->setGeometry(600,145,160,20);
    if(sht4xCommandIndex>2)
        baseHumidity->setText("Base Humidity:         "+QString::number((notifyHumidity/1000.0), 'f', 3));
    else
        baseHumidity->setText("Base Humidity:         "+QString::number((humidity/1000.0), 'f', 3));
    baseHumidity->show();


    heatingTemp->setGeometry(460,180,140,20);
    if(sht4xCommandIndex>2)
        heatingTemp->setText("Heating Temp: "+QString::number((temp/1000.0), 'f', 3));
    else
        heatingTemp->setText("Heating Temp: 0");
    heatingTemp->show();


    heatingHumidity->setGeometry(600,180,160,20);
    if(sht4xCommandIndex>2)
        heatingHumidity->setText("Hearting Humidity: "+QString::number((humidity/1000.0), 'f', 3));
    else
        heatingHumidity->setText("Hearting Humidity: 0");
    heatingHumidity->show();

    GroupBox[0]->setGeometry(450,250,320,180);
    GroupBox[0]->setTitle("param setting");
    GroupBox[0]->show();

    descriptionLabel[0]->setGeometry(460,285,120,20);
    descriptionLabel[0]->setText("Command:");
    descriptionLabel[0]->show();

    infoSHT4xComboBox[0] = new QComboBox(this);
    infoSHT4xComboBox[0]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    infoSHT4xComboBox[0]->addItem("0xFD high precision");
    infoSHT4xComboBox[0]->addItem("0xF6  medium precision");
    infoSHT4xComboBox[0]->addItem("0xE0  lowest precision");
    infoSHT4xComboBox[0]->addItem("0x39  200mW for 1s");
    infoSHT4xComboBox[0]->addItem("0x32  200mW for 0.1s");
    infoSHT4xComboBox[0]->addItem("0x2F  110mW for 1s");
    infoSHT4xComboBox[0]->addItem("0x24  110mW for 0.1s");
    infoSHT4xComboBox[0]->addItem("0x1E  20mW for 1s");
    infoSHT4xComboBox[0]->addItem("0x15  20mW for 0.1s");

    infoSHT4xComboBox[0]->setCurrentIndex(sht4xCommandIndex);
    infoSHT4xComboBox[0]->setGeometry(550,285,160,20);

    infoSHT4xComboBox[0]->show();

    connect(infoSHT4xComboBox[0],SIGNAL(currentIndexChanged(int)),this,SLOT(sht4xCommandSlelectChange(int)));



    descriptionLabel[1]->setGeometry(460,315,120,20);
    descriptionLabel[1]->setText("Time Interval:");
    descriptionLabel[1]->show();

    infoSHT4xComboBox[1] = new QComboBox(this);
    infoSHT4xComboBox[1]->setStyleSheet(
        "QComboBox {"
        "background-color: black;"

        "}"

        "QComboBox QAbstractItemView {"
        "background:black;"
        "color:white;"
        "selection-background-color:#444;"
        "}"


        "QComboBox:disabled {"
        "background-color: #E5ECF6;"
        "color: #8A8A8A;"
        "border:1px solid #C5CCD8;"
        "}"
        );


    for(int i=offsetIndex;i<61;i++)
    {
        infoSHT4xComboBox[1]->addItem(QLocale().toString(i));
    }

    infoSHT4xComboBox[1]->setGeometry(550,315,80,20);
    infoSHT4xComboBox[1]->setCurrentIndex(readoutIntervalIndex);

    infoSHT4xComboBox[1]->show();

    connect(infoSHT4xComboBox[1],SIGNAL(currentIndexChanged(int)),this,SLOT(sht4xIntervalSlelectChange(int)));


    singleReadBtn = new QPushButton(this);

    singleReadBtn->setGeometry(548,355,88,27);
    singleReadBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }

        QPushButton:disabled {
        background-color: #E5ECF6;
        color: #8A8A8A;

        }
    )");
    singleReadBtn->setText("Single read");
    singleReadBtn->show();
    connect(singleReadBtn,SIGNAL(clicked()),this,SLOT(singleReadSHT4xBtn_clicked()));

    startCheckBox = new QCheckBox("Start",this);
    startCheckBox->setGeometry(460,350,60,20);
    startCheckBox->setStyleSheet(
        "QCheckBox::indicator {"
        "width:13px;"
        "height:13px;"
        "border:1px solid #888;"
        "border-radius:3px;"
        "background:white;"
        "}"

        "QCheckBox::indicator:checked {"
        "background:#007AFF;"
        "border:1px solid #007AFF;"
        "}"

        "QCheckBox::indicator:hover {"
        "border:2px solid #007AFF;"
        "}"

        );



    startCheckBox->show();

    connect(startCheckBox, &QCheckBox::checkStateChanged,this, &MainWindow::onStateChanged);

    if(sht4xEnable)
    {
        startCheckBox->setChecked(true);
    }
    else
    {
        startCheckBox->setChecked(false);
    }


    enableLogCheckBox= new QCheckBox("Enable log",this);
    enableLogCheckBox->setGeometry(460,370,82,20);
    enableLogCheckBox->setStyleSheet(
        "QCheckBox::indicator {"
        "width:13px;"
        "height:13px;"
        "border:1px solid #888;"
        "border-radius:3px;"
        "background:white;"
        "}"

        "QCheckBox::indicator:checked {"
        "background:#007AFF;"
        "border:1px solid #007AFF;"
        "}"

        "QCheckBox::indicator:hover {"
        "border:2px solid #007AFF;"
        "}"

        );



    enableLogCheckBox->show();

    connect(enableLogCheckBox, &QCheckBox::checkStateChanged,this, &MainWindow::onLogStateChanged);

    if(sht4xEnableLog)
    {
        enableLogCheckBox->setChecked(true);
    }
    else
    {
        enableLogCheckBox->setChecked(false);
    }

    GroupBox[2]->setGeometry(450,440,320,110/*450,340,300,110*/);
    GroupBox[2]->setTitle("Serial Number");
    GroupBox[2]->show();

    descriptionLabel[2]->setGeometry(460,475,120,20);
    descriptionLabel[2]->setText("Serial number:");
    descriptionLabel[2]->show();

    infoLineEdit[0] = new QLineEdit(this);
    infoLineEdit[0]->setStyleSheet("QLineEdit { background-color: #000000; }");

    infoLineEdit[0]->setGeometry(550,475,130,20);
    infoLineEdit[0]->setMaxLength(10);
    infoLineEdit[0]->setText("0x"+QString("%1").arg(sht4xSerialNumber, 8, 16, QLatin1Char('0')).toUpper());
    //infoLineEdit[22]->setText("SN20250828AB0001");
    infoLineEdit[0]->show();

    readSHT4xSerialNumberBtn = new QPushButton(this);

    readSHT4xSerialNumberBtn->setGeometry(705,525,60,20);
    readSHT4xSerialNumberBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }
    )");
    readSHT4xSerialNumberBtn->setText("read");
    readSHT4xSerialNumberBtn->show();
    connect(readSHT4xSerialNumberBtn,SIGNAL(clicked()),this,SLOT(readSHT4xSerialNumberBtn_clicked()));

    GroupBox[3]->setGeometry(450,560,320,110/*450,220,300,110*/);
    GroupBox[3]->setTitle("soft rest");
    GroupBox[3]->show();


    setSHT4xSoftrestBtn = new QPushButton(this);

    setSHT4xSoftrestBtn->setGeometry(520,600,180,30/*510,260,180,30*/);
    setSHT4xSoftrestBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
        background-color: #AAAAAA;
        }
        QPushButton:pressed {
            background-color: #333333;
        }
    )");
    setSHT4xSoftrestBtn->setText("Soft reset");
    setSHT4xSoftrestBtn->show();
    connect(setSHT4xSoftrestBtn,SIGNAL(clicked()),this,SLOT(setSHT4xSoftrestBtn_clicked()));

    if(enableCal)
    {
        GroupBox[5]->setGeometry(780,100,320,140);
        GroupBox[5]->setTitle("Cal");
        GroupBox[5]->show();

        descriptionLabel[3]->setGeometry(790,125,120,20);
        descriptionLabel[3]->setText("Temp Cal:");
        descriptionLabel[3]->show();

        tempCalLineEdit = new QLineEdit(this);
        tempCalLineEdit->setStyleSheet("QLineEdit { background-color: #000000; }");
        tempCalLineEdit->setGeometry(870,125,80,20);
        tempCalLineEdit->setValidator(validator);
        tempCalLineEdit->setText(QString::number(tempCal,'f',2));
        tempCalLineEdit->show();

        descriptionLabel[4]->setGeometry(790,155,120,20);
        descriptionLabel[4]->setText("Humidity Cal:");
        descriptionLabel[4]->show();

        humidityCalLineEdit = new QLineEdit(this);
        humidityCalLineEdit->setStyleSheet("QLineEdit { background-color: #000000; }");
        humidityCalLineEdit->setGeometry(870,155,80,20);
        humidityCalLineEdit->setValidator(validator);
        humidityCalLineEdit->setText(QString::number(humidityCal,'f',2));
        humidityCalLineEdit->show();

        readCalBtn = new QPushButton(this);

        readCalBtn->setGeometry(790,200,70,20);
        readCalBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #000000;
                color: white;
                border-radius: 6px;
                padding: 6px 12px;
            }
            QPushButton:hover {
            background-color: #AAAAAA;
            }
            QPushButton:pressed {
                background-color: #333333;
            }

            QPushButton:disabled {
            background-color: #E5ECF6;
            color: #8A8A8A;

            }
        )");
        readCalBtn->setText("Read");
        readCalBtn->show();
        connect(readCalBtn,SIGNAL(clicked()),this,SLOT(readCalBtn_clicked()));

        saveCalBtn = new QPushButton(this);

        saveCalBtn->setGeometry(870,200,70,20);
        saveCalBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #000000;
                color: white;
                border-radius: 6px;
                padding: 6px 12px;
            }
            QPushButton:hover {
            background-color: #AAAAAA;
            }
            QPushButton:pressed {
                background-color: #333333;
            }

            QPushButton:disabled {
            background-color: #E5ECF6;
            color: #8A8A8A;

            }
        )");
        saveCalBtn->setText("Save");
        saveCalBtn->show();
        connect(saveCalBtn,SIGNAL(clicked()),this,SLOT(saveCalBtn_clicked()));
    }

}

void MainWindow::createShowRefFreqResponse()
{
    qDebug() << "[MainWindow]createShowRefFreqResponse";

    deletePage();
}

void MainWindow::createEditRefFreqResponse()
{
    qDebug() << "[MainWindow]createEditRefFreqResponse";

    deletePage();
}

void MainWindow::connectBtn_clicked()
{
    qDebug() << "[MainWindow]connectBtn_clicked";;

    if(ui->comPort_comboBox->currentIndex()==-1)
        return;

    auto serialLoc  =  ui->comPort_comboBox->currentData().toMap();

    if(checkMCP2221SerialNumberEnumerationEnable(serialLoc["VID"].toInt(),serialLoc["PID"].toInt())==false)
    {
        return;
    }

    if(initMCP2221(serialLoc["Serial"].toString(),serialLoc["VID"].toInt(),serialLoc["PID"].toInt())==false)
    {
        if(checkComPortIsMcp2221==1)
        {
            qDebug() << "[MainWindow]Not a valid MCP2221 COM port!!";
            QMessageBox::information(NULL, "MessageBox", "Not a valid MCP2221 COM port!!");
            return;
        }
    }


    if (mSerial->isOpen()) {
        qDebug() << "[MainWindow]Serial already connected, disconnecting!";
        mSerial->close();
    }

    mSerial->setPortName(serialLoc["Path"].toString());
    mSerial->setBaudRate(QSerialPort::Baud115200);
    mSerial->setDataBits(QSerialPort::Data8);
    mSerial->setParity(QSerialPort::NoParity);
    mSerial->setStopBits(QSerialPort::OneStop);
    mSerial->setFlowControl(QSerialPort::NoFlowControl);

    if(mSerial->open(QIODevice::ReadWrite)) {
        qDebug() << "[MainWindow]SERIAL: OK!";
        comPortConnect();
    } else {
        QMessageBox::information(NULL, "MessageBox", "com port open fail!!");
        qDebug() << "[MainWindow]SERIAL: ERROR!";
    }

}

void MainWindow::disconnectBtn_clicked()
{
    qDebug() << "[MainWindow]disconnectBtn_clicked";
    bAutoConnect=false;
    comPortDisconnect();

}

bool MainWindow::checkFileFormat()
{
    bool result=true;
    QString info;
    char szTemp[32];
    uint16_t  dwRead = 0;

    qDebug() << "[MainWindow]checkFileFormat";

    file.seek(FILE_APP_VERSION_INFO);

    dwRead=file.read(szTemp,32);

    if(dwRead==32)
    {
        info=QString(QLatin1String(szTemp));

        if(info.contains("FIT_V1APP_VER_R"))
        {
            imageVersion="R"+info.mid(17,2);
        }
        else
        {
            imageVersion="file error";
            result=false;
        }
    }
    else
    {
        imageVersion="file error";
        result=false;
    }

    ui->imageVersionLabel->setText("Image Version:"+imageVersion);
    qDebug() << "[MainWindow]image version:"<<imageVersion;

    return result;
}

void MainWindow::openFileBtn_clicked()
{
    qDebug() << "[MainWindow]openFileBtn_clicked";

    QSettings settings("AppConfig.ini", QSettings::IniFormat);
    QString lastPath = settings.value("FW_Upgrade/Last_Open_Dir", QDir::currentPath()).toString();
    fileName = QFileDialog::getOpenFileName(this, "Open Image",lastPath, "Bin Files (*.bin)");

    if(!fileName.isEmpty())
    {
        QFileInfo fi(fileName);
        settings.setValue("FW_Upgrade/Last_Open_Dir", fi.absolutePath());
    }

    ui->filePathLineEdit->setText(fileName);
    ui->filePathLineEdit->setCursorPosition(0);
    qDebug() << "[MainWindow]fileName:" << fileName;

    file.close();
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly);
    FileSize=file.size()-0x2000;
    if(checkFileFormat())
        file.seek(0x2000);
}

void MainWindow::updateSerialPorts()
{

    mSerialPorts = QSerialPortInfo::availablePorts();

    QList<QPair<QString, QVariantMap>> newList;
    QList<QPair<QString, QVariantMap>> existingList;

    QString currentSelection = ui->comPort_comboBox->currentText();

    for (int i = 0; i < ui->comPort_comboBox->count(); ++i) {

        QString text = ui->comPort_comboBox->itemText(i);
        QVariantMap data = ui->comPort_comboBox->itemData(i).toMap();

        existingList.append(qMakePair(text, data));
    }

    for (QSerialPortInfo &port : mSerialPorts) {

        LogManager::instance()->log(LogLevel::UartInfo, QString("[MainWindow]Port: %1").arg(port.portName()));
        LogManager::instance()->log(LogLevel::UartInfo, QString("[MainWindow]Description: %1").arg(port.description()));
        LogManager::instance()->log(LogLevel::UartInfo, QString("[MainWindow]Manufacturer: %1").arg(port.manufacturer()));
        LogManager::instance()->log(LogLevel::UartInfo, QString("[MainWindow]Serial: %1").arg(port.serialNumber()));
        LogManager::instance()->log(LogLevel::UartInfo, QString("[MainWindow]VID: %1").arg(port.vendorIdentifier()));
        LogManager::instance()->log(LogLevel::UartInfo, QString("[MainWindow]PID: %1").arg(port.productIdentifier()));

        WCHAR lpTargetPath[5000];
        if(QueryDosDevice( (const wchar_t*)port.portName().utf16(), (LPWSTR)lpTargetPath, 5000)!=0)
        {
            QVariantMap portInfo;

            portInfo["Serial"]=port.serialNumber();
            portInfo["Path"]=port.systemLocation();
            portInfo["VID"]=port.vendorIdentifier();
            portInfo["PID"]=port.productIdentifier();
            newList.append(qMakePair(port.portName(), portInfo));
        }

    }

    if(existingList!=newList)
    {
        ui->comPort_comboBox->blockSignals(true);  // 避免觸發 onChanged 事件

        ui->comPort_comboBox->clear();

        autoConnectPortName.clear();

        for (int i = 0; i < newList.size(); ++i) {
            const auto& pair = newList.at(i);

            if(bAutoConnect&&!IsConnect)
            {
                qDebug() << "[MainWindow]Detect VID:" << pair.second["VID"].toInt() << "ini VID:" << Auto_Connect_VID.toInt(NULL,16);
                qDebug() << "[MainWindow]Detect PID:" << pair.second["PID"].toInt() << "ini PID:" << Auto_Connect_PID.toInt(NULL,16);

                if(pair.second["VID"].toInt()==Auto_Connect_VID.toInt(NULL,16)&&pair.second["PID"].toInt()==Auto_Connect_PID.toInt(NULL,16))
                {
                    qDebug() << "[MainWindow]find autoConnect VID&PID";
                    if(!Auto_Connect_Serial_Number.isEmpty())
                    {
                        if(pair.second["Serial"].toString()==Auto_Connect_Serial_Number)
                        {
                            qDebug() << "[MainWindow]find autoConnect Serial number";
                            autoConnectPortName=pair.first;
                            currentSelection=pair.first;
                            qDebug() << "[MainWindow]autoConnectPortName:"<<autoConnectPortName;
                        }
                        else
                        {
                            qDebug() << "[MainWindow]not find autoConnect Serial number";
                        }
                    }
                    else
                    {
                        autoConnectPortName=pair.first;
                        currentSelection=pair.first;
                        qDebug() << "[MainWindow]autoConnectPortName:"<<autoConnectPortName;
                    }
                }
            }
            ui->comPort_comboBox->addItem(pair.first, pair.second);
        }

        int index = ui->comPort_comboBox->findText(currentSelection);
        if (index != -1) {
            ui->comPort_comboBox->setCurrentIndex(index);
        }
        else
        {
            if(mSerial->portName()==currentSelection)
            {
                comPortDisconnect();
            }
        }
        bAutoConnect=true;
        ui->comPort_comboBox->blockSignals(false);
    }

    if(!IsConnect&&bAutoConnect)
    {
        if(!autoConnectPortName.isEmpty())
        {
            ui->comPort_comboBox->setCurrentText(autoConnectPortName);
            qDebug() << "[MainWindow]comPort_comboBox Index:"<<ui->comPort_comboBox->currentIndex();
            connectBtn_clicked();
        }
    }




}


int MainWindow::SendCMD(char* szMsg, uint16_t nCmdLen)
{
    uint16_t numWritten;
    QByteArray data;
    for(int i=0;i<nCmdLen;i++)
    {
        data.append(szMsg[i]);
    }
    //qDebug() << "Send data"<<data;
    numWritten=mSerial->write(data);

    return numWritten;
}

uint8_t MainWindow::calculateChecksum(uint8_t *data,uint8_t len)
{
    uint8_t sum=0;

    for(int i=0;i<len;i++)
    {
        //qDebug() << "data"<<data[i];
        sum+=data[i];
        //qDebug() << "sum"<<sum;
    }


    return (uint8_t)(0x100-sum);
}

void MainWindow::onCheckFunctionCalled(int count)
{
    //qDebug() << "[MainWindow] Check function is called" << count;
    Q_UNUSED(count);
    ProcessPacket();

    if(mSerialScanTimer.elapsed()>=1000)
    {
        /*if(IsConnect)
        {
            if(bCheckDeviceOffLine)
            {
                if(!bReSyncDataWhenDeviceOnLine)
                {
                    bReSyncDataWhenDeviceOnLine=true;
                    //ui->temperatureLabel->setText("Temperature:");
                    //ui->humidtyLabel->setText("Humidty:");
                    m_sht4xCard->clearValue1();
                    m_sht4xCard->clearValue2();
                    ui->deviceIDlabel->setText("");
                    m_deviceidCard->clearDeviceID();
                    //setWindowTitle(AppVersion+" Device ID:");
                }
            }
            else
            {
                if(bReSyncDataWhenDeviceOnLine)
                {
                    bReSyncDataWhenDeviceOnLine=false;
                    syncDeviceData();
                }
            }
        }*/

        updateSerialPorts();
        mSerialScanTimer.restart();
    }
}

void MainWindow::onWorkFinished()
{
    qDebug() << "[MainWindow] Mission accomplished！";
}

void MainWindow::serialReadyRead()
{
    QByteArray data = mSerial->readAll();
    QStringList debugList;

    for (uint16_t i = 0; i <data.length(); i++)
    {
        ReceivePacket[DATA_BUF_INDEX] = data[i];

        debugList << "0x"+QString("%1").arg((uint8_t)ReceivePacket[DATA_BUF_INDEX], 2, 16, QLatin1Char('0')).toUpper();

        DATA_BUF_INDEX += 1;

        if (DATA_BUF_INDEX >= 1024)
            DATA_BUF_INDEX = 0;
    }

    if(IsBufferHasData())
        workerThread->wake();

    LogManager::instance()->log(LogLevel::UartRecv, QString("[MainWindow]recv length: %1 recv packet: %2")
                                                        .arg(data.length())
                                                        .arg(debugList.join(", ")));
}


void MainWindow::GenCommand(uint8_t nCMD, uint8_t *pCMD, uint16_t nCmdLen)
{
    int n_Len = 0;
    char m_Linkbuffer_Out[256];

    qDebug() << "[MainWindow]GenCommand";

    if(!IsConnect)
    {
        qDebug() << "[MainWindow]COM Port is not connected.";
        return;
    }

    m_Linkbuffer_Out[0] = (uint8_t)0x55;
    m_Linkbuffer_Out[1] = (uint8_t)nCmdLen + 1;
    m_Linkbuffer_Out[2] = (uint8_t)nCMD & 0xff;
    if (nCmdLen>0 && pCMD != 0)
        memcpy(m_Linkbuffer_Out + 3, pCMD, nCmdLen);

    n_Len = nCmdLen + 3;

    if (n_Len != SendCMD(m_Linkbuffer_Out, n_Len))
    {
        qDebug() << "[MainWindow]Writing to the serial port timed out";
    }
}

uint8_t MainWindow::HandlePacket()
{
    uint8_t ret = 0;
    bCheckDeviceOffLine=false;
    if(devID==1)
        resetCheckDisconnectFlag();
    if (CMD_Buffer[PAYLOADLEN] == 0x00)
    {
        // for Large Packet Format
    }
    else
    {
        //qDebug() <<"[MainWindow]HandlePacket CMDID:"<<CMD_Buffer[CMDID];
        // for Small Packet Format
        switch (CMD_Buffer[CMDID])
        {
            case ACK_CMD:

                if (CMD_Buffer[CMDDATA] == OTA_UPGRADE_INIT_CMD)
                {
                    if(!bUpgrade||!file.exists())
                        return ret;

                    if (CMD_Buffer[CMDDATA + 1] != CMD_SUCCESS)
                    {
                        qDebug() <<"[MainWindow]Enter Upgrade mode Fail!!";
                        QMessageBox::information(NULL, "MessageBox", "Upgrade Init Fail!!");
                        bUpgrade=false;
                        ui->upgradeBtn->setEnabled(true);
                        ui->tabWidget->setTabEnabled(0,true);
                        ui->tabWidget->setTabEnabled(1,true);
                        ui->tabWidget->setTabEnabled(2,true);
                        ui->tabWidget->setTabEnabled(3,true);
                        ui->tabWidget->setTabEnabled(4,true);
                    }
                    else
                    {
                        uint16_t  dwRead = 0;
                        char szTemp[128];
                        ImageCheckSum = 0;

                        dwRead=file.read(szTemp,128);

                        for (uint16_t i = 0; i < dwRead; i++)
                            ImageCheckSum += (uint8_t)szTemp[i];

                        GenCommand(OTA_UPGRADE_START_CMD, (uint8_t*) szTemp, (uint16_t)dwRead);

                        UpgradePos += dwRead;
                        ui->progressBar->setValue(UpgradePos / (FileSize/100));
                    }

                }
                else if (CMD_Buffer[CMDDATA] == 0xB1)
                {
                    if (CMD_Buffer[CMDDATA + 1] != CMD_SUCCESS)
                    {
                        qDebug() <<"[MainWindow]Enter Upgrade mode Fail!!";
                        QMessageBox::information(NULL, "MessageBox", "Enter Upgrade mode Fail!!");
                        bUpgrade=false;
                        ui->upgradeBtn->setEnabled(true);
                        ui->tabWidget->setTabEnabled(0,true);
                        ui->tabWidget->setTabEnabled(1,true);
                        ui->tabWidget->setTabEnabled(2,true);
                        ui->tabWidget->setTabEnabled(3,true);
                        ui->tabWidget->setTabEnabled(4,true);
                    }
                    else
                    {
                        GenCommand(0xAA, NULL, 0);
                        bSend0xB0CommandAfterReset=true;

                    }
                }
                else if (CMD_Buffer[CMDDATA] == OTA_UPGRADE_START_CMD)
                {

                    if(!bUpgrade||!file.exists())
                        return ret;

                    if (CMD_Buffer[CMDDATA + 1] != CMD_SUCCESS)
                    {
                        qDebug() <<"[MainWindow]Upgrade Fail!!";
                        QMessageBox::information(NULL, "MessageBox", "Upgrade Fail1!!");
                        bUpgrade=false;
                        ui->upgradeBtn->setEnabled(true);
                        ui->tabWidget->setTabEnabled(0,true);
                        ui->tabWidget->setTabEnabled(1,true);
                        ui->tabWidget->setTabEnabled(2,true);
                        ui->tabWidget->setTabEnabled(3,true);
                        ui->tabWidget->setTabEnabled(4,true);
                    }
                    else
                    {
                        uint16_t  dwRead = 0;
                        char szTemp[128];

                        dwRead=file.read(szTemp,128);

                        if (dwRead <= 0)
                        {
                            szTemp[0] = (FileSize & 0xFF000000) >> 24;
                            szTemp[1] = (FileSize & 0x00FF0000) >> 16;
                            szTemp[2] = (FileSize & 0x0000FF00) >> 8;
                            szTemp[3] = (FileSize & 0x000000FF);

                            szTemp[4] = (ImageCheckSum & 0xFF000000) >> 24;
                            szTemp[5] = (ImageCheckSum & 0x00FF0000) >> 16;
                            szTemp[6] = (ImageCheckSum & 0x0000FF00) >> 8;
                            szTemp[7] = (ImageCheckSum & 0x000000FF);

                            qDebug() <<"[MainWindow]FileSize:"<<FileSize;
                            qDebug() <<"[MainWindow]ImageCheckSum:"<<ImageCheckSum;
                            GenCommand(OTA_UPGRADE_END_CMD, (uint8_t*)szTemp, 8);
                        }
                        else
                        {
                            for (uint16_t i = 0; i < dwRead; i++)
                                ImageCheckSum += (uint8_t)szTemp[i];

                            GenCommand(OTA_UPGRADE_START_CMD, (uint8_t*)szTemp, (uint16_t)dwRead);

                            UpgradePos += dwRead;
                            ui->progressBar->setValue(UpgradePos / (FileSize/100));
                        }
                    }


                }
                else if (CMD_Buffer[CMDDATA] == OTA_UPGRADE_END_CMD)
                {
                    if (CMD_Buffer[CMDDATA + 1] == CMD_SUCCESS)
                    {
                        GenCommand(0xAA, NULL, 0);
                        qDebug() <<"[MainWindow]Upgrade Success!!";
                        QMessageBox::information(NULL, "MessageBox", "Upgrade Success!!");
                    }
                    else
                    {
                        GenCommand(0xAA, NULL, 0);
                        qDebug() <<"[MainWindow]Upgrade Fail!!";
                        QMessageBox::information(NULL, "MessageBox", "Upgrade Fail!!");
                    }
                    bUpgrade=false;
                    ui->upgradeBtn->setEnabled(true);
                    ui->tabWidget->setTabEnabled(0,true);
                    ui->tabWidget->setTabEnabled(1,true);
                    ui->tabWidget->setTabEnabled(2,true);
                    ui->tabWidget->setTabEnabled(3,true);
                    ui->tabWidget->setTabEnabled(4,true);
                    GenCommand(0x01, NULL, 0);
                }
                else if (CMD_Buffer[CMDDATA] == WRITE_EEPROM_BLOCK_CMD)
                {
                    if (CMD_Buffer[CMDDATA + 1] == CMD_CHECKSUMERROR)
                    {
                        if(bSaveAllFlag)
                        {
                            if(writeEEPROMprogress)
                            {
                                writeEEPROMprogress->close();

                                delete writeEEPROMprogress;

                                writeEEPROMprogress = nullptr;
                            }
                            resetSaveAllTimeout();
                            bSaveAllFlag=false;
                        }
                        qDebug() <<"[MainWindow]Save Checksum Fail!!";
                        QMessageBox::information(NULL, "MessageBox", "Save Checksum Fail!!");

                    }
                    else if (CMD_Buffer[CMDDATA + 1] == CMD_PAGE0_WRITE_SUCCESS)
                    {
                        if(bSaveAllFlag)
                        {
                             writeEEPROMprogress->setLabelText(QString("Writing Page%1...").arg(2));

                             writeEEPROMprogress->setValue(1);

                             resetSaveAllTimeout();

                             savePage2();
                        }
                        else
                        {
                            GenCommand(0xAA, NULL, 0);
                            qDebug() <<"[MainWindow]Save Success!!";
                            QMessageBox::information(NULL, "MessageBox", "Save Page0 Success!!");
                        }

                    }
                    else if (CMD_Buffer[CMDDATA + 1] == CMD_PAGE2_WRITE_SSUCCESS)
                    {
                        if(bSaveAllFlag)
                        {
                            writeEEPROMprogress->setLabelText(QString("Writing Page%1...").arg(3));

                            writeEEPROMprogress->setValue(2);

                            resetSaveAllTimeout();

                            savePage3();
                        }
                        else
                        {
                            GenCommand(0xAA, NULL, 0);
                            qDebug() <<"[MainWindow]Save Success!!";
                            QMessageBox::information(NULL, "MessageBox", "Save Page2 Success!!");
                        }

                    }
                    else if (CMD_Buffer[CMDDATA + 1] == CMD_PAGE3_WRITE_SSUCCESS)
                    {

                        if(bSaveAllFlag)
                        {
                            writeEEPROMprogress->setLabelText(QString("Writing Page%1...").arg(4));

                            writeEEPROMprogress->setValue(3);

                            resetSaveAllTimeout();

                            savePage4();
                        }
                        else
                        {
                            GenCommand(0xAA, NULL, 0);
                            qDebug() <<"[MainWindow]Save Success!!";
                            QMessageBox::information(NULL, "MessageBox", "Save Page3 Success!!");
                        }

                    }
                    else if (CMD_Buffer[CMDDATA + 1] == CMD_PAGE4_WRITE_SSUCCESS)
                    {
                        if(bSaveAllFlag)
                        {

                            writeEEPROMprogress->setValue(4);

                            if(writeEEPROMprogress)
                            {
                                writeEEPROMprogress->close();

                                delete writeEEPROMprogress;

                                writeEEPROMprogress = nullptr;
                            }

                            resetSaveAllTimeout();
                            bSaveAllFlag=false;

                            GenCommand(0xAA, NULL, 0);
                            QMessageBox::information(this,"MessageBox","Save All completed");
;
                        }
                        else
                        {
                            GenCommand(0xAA, NULL, 0);
                            qDebug() <<"[MainWindow]Save Success!!";
                            QMessageBox::information(NULL, "MessageBox", "Save Page4 Success!!");
                        }

                    }
                    else if (CMD_Buffer[CMDDATA + 1] == CMD_INFO1_WRITE_SSUCCESS)
                    {
                        if(burnInfo)
                            WriteInfoSuccessIncreaseSerialNumberOffset();

                        recordWriteInfo();

                        qDebug() <<"[MainWindow]Write Info Success!!";
                        //if(bRefreshInfoBlock1)
                        //{
                            refreshInfo();
                            //bRefreshInfoBlock1=false;
                        //}

                        QMessageBox::information(NULL, "MessageBox", "Write Info Success!!");

                        if(ui->tabWidget->currentIndex()==4)
                        {
                            infoBtnGrup->button(0)->setChecked(true);
                            onTabChanged(4);
                        }
                    }
                    else if (CMD_Buffer[CMDDATA + 1] == CMD_INFO2_WRITE_SSUCCESS)
                    {
                        recordWriteNote();

                        qDebug() <<"[MainWindow]Write Note Success!!";
                        //if(bRefreshInfoBlock2)
                        //{
                            refreshNote();
                            //bRefreshInfoBlock1=false;
                        //}

                        QMessageBox::information(NULL, "MessageBox", "Write Info Success!!");

                        if(ui->tabWidget->currentIndex()==4)
                        {
                            infoBtnGrup->button(0)->setChecked(true);
                            onTabChanged(4);
                        }
                    }
                    else
                    {

                        qDebug() <<"[MainWindow]Save All Fail!!";
                        QMessageBox::information(NULL, "MessageBox", "Save All Fail!!");
                    }
                }
                else if (CMD_Buffer[CMDDATA] == WRITE_PCMD_BLOCK_CMD)
                {
                    if (CMD_Buffer[CMDDATA + 1] != CMD_SUCCESS)
                    {
                        if (CMD_Buffer[CMDDATA + 1] == CMD_CHECKSUMERROR)
                        {
                            qDebug() <<"[MainWindow]Write All Checksum Fail!!";
                            QMessageBox::information(NULL, "MessageBox", "Write All Checksum Fail!!");
                        }
                        else
                        {
                            qDebug() <<"[MainWindow]Write All Fail!!";
                            QMessageBox::information(NULL, "MessageBox", "Write All Fail!!");
                        }
                    }
                    else
                    {
                        qDebug() <<"[MainWindow]Write All Success!!";
                        QMessageBox::information(NULL, "MessageBox", "Write All Success!!");
                    }
                }
                else if (CMD_Buffer[CMDDATA] == WRITE_PCMD_REG_WITH_PAGE_PARAMETERS_CMD)
                {
                    if (CMD_Buffer[CMDDATA + 1] != CMD_SUCCESS)
                    {
                        qDebug() <<"[MainWindow]Write REG Fail!!";
                        QMessageBox::information(NULL, "MessageBox", "Write REG Fail!!");
                    }
                    else
                    {
                        qDebug() <<"[MainWindow]Write REG Success!!";
                        QMessageBox::information(NULL, "MessageBox", "Write REG Success!!");
                    }
                }
                /*else if (CMD_Buffer[CMDDATA] == 0x09)
                {
                    if (CMD_Buffer[CMDDATA + 1] != CMD_SUCCESS)
                    {
                        QMessageBox::information(NULL, "MessageBox", "Reset to Default Fail!!");
                    }
                    else
                    {
                        GenCommand(0xAA, NULL, 0);
                        QMessageBox::information(NULL, "MessageBox", "Reset to Default Success!!");
                    }
                }*/
                else if (CMD_Buffer[CMDDATA] == 0xAA)
                {
                    qDebug() <<"[MainWindow]Device reset success!!";
                    if(bSend0xB0CommandAfterReset)
                    {
                        bSend0xB0CommandAfterReset=false;
                        GenCommand(0xB0, NULL, 0);

                    }
                    else if(bReSyncAfterReset)
                    {
                       qDebug() <<"[MainWindow]bReSyncAfterReset!!";
                       bReSyncAfterReset=false;
                       setReconnectFlag();

                    }
                }
                else if (CMD_Buffer[CMDDATA] == 0xF1)
                {
                    if (CMD_Buffer[CMDDATA + 1] == CMD_SUCCESS)
                    {
                        bReSyncAfterReset=true;
                        GenCommand(0xAA, NULL, 0);
                        //qDebug() <<"[MainWindow]Watchdog flage change!!";
                        QMessageBox::information(NULL, "MessageBox", "Watchdog flag change reset device!!");
                    }
                }
                else if (CMD_Buffer[CMDDATA] == SHT4X_SOFT_RESET_CMD)
                {
                    if (CMD_Buffer[CMDDATA + 1] == CMD_SUCCESS)
                    {
                        QMessageBox::information(NULL, "MessageBox", "SHT4x reset success!!");
                    }
                    else
                    {
                        QMessageBox::information(NULL, "MessageBox", "SHT4x reset fail!!");
                    }
                    recordSHT4x(1);
                }
                else if (CMD_Buffer[CMDDATA] == WRITE_SHT4X_CAL_CMD)
                {
                    if (CMD_Buffer[CMDDATA + 1] == CMD_SUCCESS)
                    {
                        QMessageBox::information(NULL, "MessageBox", "Save Cal Success!!");
                    }
                    else
                    {
                        QMessageBox::information(NULL, "MessageBox", "Save Cal fail!!");
                    }
                }
                else if (CMD_Buffer[CMDDATA] == WRITE_SBM100_BLOCK_CMD)
                {
                    if (CMD_Buffer[CMDDATA + 1] == CMD_SUCCESS)
                    {
                        GenCommand(0xAA, NULL, 0);
                        QMessageBox::information(NULL, "MessageBox", "write Success!!");
                    }
                    else  if (CMD_Buffer[CMDDATA + 1] == 0x02)
                    {
                        QMessageBox::information(NULL, "MessageBox", "write fail,SMB100 is not exist!!");
                    }
                    else
                    {
                        QMessageBox::information(NULL, "MessageBox", "write fail!!");
                    }
                }
            break;
            case GET_FW_VERSION_RESPONSE_CMD:
            {
                //resetCheckDisconnectFlag();
                devID=0xff;
                if(CMD_Buffer[CMDDATA]==0xff)
                {
                    version=CMD_Buffer[CMDDATA+1];
                    ui->fwVerionLabel->setText("The App has been erased and the device is now\r\nin bootloader mode.BL Version:"+QString::asprintf("%02d",version));
                }
                else
                {
                    version=CMD_Buffer[CMDDATA]<<8|CMD_Buffer[CMDDATA+1];
                    ui->fwVerionLabel->setText("Device FW Version:R"+QString::asprintf("%02d",version));
                }
                if(CMD_Buffer[PAYLOADLEN]<4)
                {
                    devID=0x01;
                }
                else
                {
                    devID=CMD_Buffer[CMDDATA+2];
                }
                qDebug() <<"[MainWindow]GET_FW_VERSION_RESPONSE_CMD Version:"<<version;
                qDebug() <<"[MainWindow]devID:"<<devID;
            }
            break;
            case READ_SBM100_BLOCK_RESPONSE_CMD:
            {
                if(CMD_Buffer[CMDDATA+1]==1)
                {
                    uint16_t tempRegValue;
                    isSBM100Exist=true;
                    infoRadioButton[6]->setEnabled(true);
                    //infoRadioButton[6]->setVisible(true);

                    tempRegValue=CMD_Buffer[CMDDATA+2]<<8|CMD_Buffer[CMDDATA+3];

                    dcb_cutoff=tempRegValue&0x0007;
                    dcb_bypass=(tempRegValue&0x0010)>>4;

                    tempRegValue=CMD_Buffer[CMDDATA+4]<<8|CMD_Buffer[CMDDATA+5];

                    biquad_b0=tempRegValue;

                    tempRegValue=CMD_Buffer[CMDDATA+6]<<8|CMD_Buffer[CMDDATA+7];

                    biquad_b1=tempRegValue;

                    tempRegValue=CMD_Buffer[CMDDATA+8]<<8|CMD_Buffer[CMDDATA+9];

                    biquad_b2=tempRegValue;

                    tempRegValue=CMD_Buffer[CMDDATA+10]<<8|CMD_Buffer[CMDDATA+11];

                    biquad_a1=tempRegValue;

                    tempRegValue=CMD_Buffer[CMDDATA+12]<<8|CMD_Buffer[CMDDATA+13];

                    biquad_a2=tempRegValue;

                    tempRegValue=CMD_Buffer[CMDDATA+14]<<8|CMD_Buffer[CMDDATA+15];

                    biquad_coeff_write=tempRegValue&0x0001;

                    tempRegValue=CMD_Buffer[CMDDATA+16]<<8|CMD_Buffer[CMDDATA+17];

                    biquad_bypass=tempRegValue&0x0001;

                    tempRegValue=CMD_Buffer[CMDDATA+18]<<8|CMD_Buffer[CMDDATA+19];

                    out24not28=(tempRegValue&(1<<6))>>6;
                    polarity=(tempRegValue&(1<<5))>>5;
                    user_gain=tempRegValue&0x1F;

                    if(ui->tabWidget->currentIndex()==4)
                    {
                        onTabChanged(4);
                    }
                }
                else
                {
                    isSBM100Exist=false;
                    infoRadioButton[6]->setEnabled(false);
                    if(infoBtnGrup->checkedId()==6)
                    {
                        infoBtnGrup->button(0)->setChecked(true);

                        if(ui->tabWidget->currentIndex()==4)
                            onTabChanged(4);
                    }
                }
            }
            break;
            case READ_DPS368_COEFF_RESPONSE_CMD:
            {

                if(CMD_Buffer[CMDDATA]==0x01)
                {
                    initCoeff(&CMD_Buffer[CMDDATA+1]);
                    isDSP368Exist=true;
                    infoRadioButton[5]->setEnabled(true);
                    //infoRadioButton[5]->setVisible(true);
                }
                else
                {
                    isDSP368Exist=false;
                    //ui->pressureLabel->setText("Pressure:");
                    //ui->tempDPS368Label->setText("Temperature:");
                    m_dsp368Card->clearValue1();
                    m_dsp368Card->clearValue2();

                    infoRadioButton[5]->setEnabled(false);
                    if(infoBtnGrup->checkedId()==5)
                    {
                        infoBtnGrup->button(0)->setChecked(true);

                        if(ui->tabWidget->currentIndex()==4)
                            onTabChanged(4);
                    }
                }
            }
            break;
            case READ_DPS368_TEMP_AND_PRESSURE_RESPONS_CMD:
            {
                bWaitDsp368ReadOutResponse=false;

                int32_t tempRaw=(uint32_t)CMD_Buffer[CMDDATA] << 16 | (uint32_t)CMD_Buffer[CMDDATA+1] << 8 | (uint32_t)CMD_Buffer[CMDDATA+2];
                int32_t pressureRaw=(uint32_t)CMD_Buffer[CMDDATA+3] << 16 | (uint32_t)CMD_Buffer[CMDDATA+4] << 8 | (uint32_t)CMD_Buffer[CMDDATA+5];
                int responseTempOSR=CMD_Buffer[CMDDATA+6];
                int responsePressureOSR=CMD_Buffer[CMDDATA+7];


                qDebug() <<"[MainWindow]responseTempOSR:"<<responseTempOSR;
                qDebug() <<"[MainWindow]responsePressureOSR:"<<responsePressureOSR;

                getTwosComplement(&tempRaw, 24);
                getTwosComplement(&pressureRaw, 24);

                dsp368_temp=calcTemp(tempRaw,responseTempOSR);
                dsp368_pressure=calcPressure(pressureRaw,responsePressureOSR);


                qDebug() <<"[MainWindow]READ_DPS368_TEMP_AND_PRESSURE_RESPONS_CMD temperatureLabel:"<<dsp368_temp;
                qDebug() <<"[MainWindow]READ_DPS368_TEMP_AND_PRESSURE_RESPONS_CMD pressureLabel:"<<dsp368_pressure;

                //ui->pressureLabel->setText("Pressure:"+QLocale().toString(dsp368_pressure));
                //ui->tempDPS368Label->setText("Temperature:"+QLocale().toString(dsp368_temp));

                m_dsp368Card->setValue1(dsp368_temp);
                m_dsp368Card->setValue2(dsp368_pressure);

                recordDSP368(0);
            }
            break;
            case SHT4X_NOTIFY_CMD:
            {
                notifyTemp=CMD_Buffer[CMDDATA]<<24|CMD_Buffer[CMDDATA+1]<<16|CMD_Buffer[CMDDATA+2]<<8|CMD_Buffer[CMDDATA+3];
                notifyHumidity=CMD_Buffer[CMDDATA+4]<<24|CMD_Buffer[CMDDATA+5]<<16|CMD_Buffer[CMDDATA+6]<<8|CMD_Buffer[CMDDATA+7];

                recordSHT4xNotify();

                if(baseTemp)
                    baseTemp->setText("Base Temp:       "+QLocale().toString(notifyTemp/1000.0));

                if(baseHumidity)
                    baseHumidity->setText("Base Humidity:         "+QLocale().toString(notifyHumidity/1000.0));

                if((notifyTemp/1000.0)>=heatingProtectionTemperature)
                {


                    sht4xCommandIndex=0;
                    if(infoSHT4xComboBox[0])
                    {
                        infoSHT4xComboBox[0]->setCurrentIndex(sht4xCommandIndex);
                    }
                    else
                    {
                        QSettings settings("AppConfig.ini", QSettings::IniFormat);
                        settings.setValue("SHT4x/CommandType",sht4xCommandIndex);
                    }


                    QString msg="Temperature greater than "+QLocale().toString(heatingProtectionTemperature)+"\r\nStop heating!!\r\nCommand change to 0xFD high precision";
                    QMessageBox::information(NULL, "MessageBox", msg);
                }

            }
            break;
            case GET_TH_RESPONSE_CMD:
            {

                bWaitReadOutResponse=false;
                if(CMD_Buffer[1]==9)
                {
                    temp=CMD_Buffer[CMDDATA]<<24|CMD_Buffer[CMDDATA+1]<<16|CMD_Buffer[CMDDATA+2]<<8|CMD_Buffer[CMDDATA+3];
                    humidity=CMD_Buffer[CMDDATA+4]<<24|CMD_Buffer[CMDDATA+5]<<16|CMD_Buffer[CMDDATA+6]<<8|CMD_Buffer[CMDDATA+7];

                    //ui->temperatureLabel->setText("Temperature:"+QLocale().toString(temp/1000.0));
                    //ui->humidtyLabel->setText("Humidty:"+QLocale().toString(humidity/1000.0));
                    m_sht4xCard->setValue1(temp/1000.0);
                    m_sht4xCard->setValue2(humidity/1000.0);


                }
                else
                {
                    uint16_t tempRaw=CMD_Buffer[CMDDATA]<<8|CMD_Buffer[CMDDATA+1];
                    uint16_t humidityRaw=CMD_Buffer[CMDDATA+2]<<8|CMD_Buffer[CMDDATA+3];
                    uint8_t isValid=CMD_Buffer[CMDDATA+4];

                    /**
                     * formulas for conversion of the sensor signals, optimized for fixed point
                     * algebra:
                     * Temperature = 175 * S_T / 65535 - 45
                     * Relative Humidity = 125 * (S_RH / 65535) - 6
                     */

                    qDebug() <<"[MainWindow]GET_TH_RESPONSE_CMD isValid:"<<isValid;
                    if(isValid==0)
                        return 0;

                    temp = ((21875 * (int32_t)tempRaw) >> 13) - 45000;
                    humidity = ((15625 * (int32_t)humidityRaw) >> 13) - 6000;


                    if(sht4xEnable||bSingleRead)
                    {
                        recordSHT4x(0);

                        if(enableCal)
                        {
                            //ui->temperatureLabel->setText("Temperature:"+QLocale().toString((temp/1000.0)+tempCal));
                            //ui->humidtyLabel->setText("Humidty:"+QLocale().toString((humidity/1000.0)+humidityCal));
                            m_sht4xCard->setValue1((temp/1000.0)+tempCal);
                            m_sht4xCard->setValue2((humidity/1000.0)+humidityCal);
                        }
                        else
                        {
                            //ui->temperatureLabel->setText("Temperature:"+QLocale().toString(temp/1000.0));
                            //ui->humidtyLabel->setText("Humidty:"+QLocale().toString(humidity/1000.0));

                            m_sht4xCard->setValue1(temp/1000.0);
                            m_sht4xCard->setValue2(humidity/1000.0);
                        }

                        bSingleRead=false;
                    }
                    else
                    {
                        //ui->temperatureLabel->setText("Temperature:");
                        //ui->humidtyLabel->setText("Humidty:");
                        m_sht4xCard->clearValue1();
                        m_sht4xCard->clearValue2();


                    }

                    if(sht4xCommandIndex>2)
                    {
                        if(heatingTemp)
                        {
                            heatingTemp->setText("Heating Temp: "+QLocale().toString(temp/1000.0));
                        }

                        if(heatingHumidity)
                        {
                            heatingHumidity->setText("Hearting Humidity: "+QLocale().toString(humidity/1000.0));

                        }
                    }
                    else
                    {
                        if(baseTemp)
                            baseTemp->setText("Base Temp:       "+QLocale().toString(temp/1000.0));

                        if(baseHumidity)
                            baseHumidity->setText("Base Humidity:         "+QLocale().toString(humidity/1000.0));

                    }
                    qDebug() <<"[MainWindow]GET_TH_RESPONSE_CMD tempRaw:"<<tempRaw;
                    qDebug() <<"[MainWindow]GET_TH_RESPONSE_CMD humidityRaw:"<<humidityRaw;

                }

                qDebug() <<"[MainWindow]GET_TH_RESPONSE_CMD Len:"<<CMD_Buffer[1];
                qDebug() <<"[MainWindow]GET_TH_RESPONSE_CMD temperatureLabel:"<<temp;
                qDebug() <<"[MainWindow]GET_TH_RESPONSE_CMD humidtyLabel:"<<humidity;

            }
            break;
            case GET_SHT4X_SERIAL_NUM_RESPONSE_CMD:
            {
                sht4xSerialNumber=CMD_Buffer[CMDDATA]<<24|CMD_Buffer[CMDDATA+1]<<16|CMD_Buffer[CMDDATA+2]<<8|CMD_Buffer[CMDDATA+3];
                qDebug() <<"[MainWindow]GET_SHT4X_SERIAL_NUM_RESPONSE_CMD sht4xSerialNumber:"<<sht4xSerialNumber;
                if(infoLineEdit[0])
                {
                    infoLineEdit[0]->setText("0x"+QString("%1").arg(sht4xSerialNumber, 8, 16, QLatin1Char('0')).toUpper());
                }
            }
            break;
            case READ_SHT4X_CAL_RESPONSE_CMD:
            {

                memcpy(&tempCal,&CMD_Buffer[CMDDATA],4);
                memcpy(&humidityCal,&CMD_Buffer[CMDDATA+4],4);

                if(tempCal!=tempCal)
                {
                    tempCal=0;
                }

                if(humidityCal!=humidityCal)
                {
                    humidityCal=0;
                }

                if(tempCalLineEdit)
                    tempCalLineEdit->setText(QString::number(tempCal,'f',2));

                if(humidityCalLineEdit)
                    humidityCalLineEdit->setText(QString::number(humidityCal,'f',2));

                qDebug() <<"[MainWindow]READ_SHT4X_CAL_RESPONSE_CMD tempCal:"<<tempCal;
                qDebug() <<"[MainWindow]READ_SHT4X_CAL_RESPONSE_CMD humidityCal:"<<humidityCal;
            }
            case READ_EEPROM_BLOCK_RESPONSE_CMD:
            {
                uint8_t len=CMD_Buffer[PAYLOADLEN];
                uint8_t checksum=calculateChecksum(&CMD_Buffer[CMDDATA+1],len-3);

                qDebug() <<"[MainWindow]READ_EEPROM_BLOCK_RESPONSE_CMD checksum:"<<checksum<<CMD_Buffer[len+1]<<len<<" BlockID:"<<CMD_Buffer[CMDDATA];

                if(checksum==CMD_Buffer[len+1])
                {
                    if(CMD_Buffer[CMDDATA]==BLOCK_ID_PAGE0)
                    {
                        for(int i=0;i<len-3;i++)
                        {
                            QString value="0x"+QString("%1").arg(CMD_Buffer[CMDDATA+1+i], 2, 16, QLatin1Char('0')).toUpper();
                            ui->tableWidget_page0->item(i,2)->setText(value);
                        }
                        if(ui->tabWidget->currentIndex()==0)
                        {
                          onTabChanged(0);
                        }
                    }
                    else if(CMD_Buffer[CMDDATA]==BLOCK_ID_PAGE2)
                    {
                        for(int i=1;i<len-3;i++)
                        {
                            QString value="0x"+QString("%1").arg(CMD_Buffer[CMDDATA+1+i], 2, 16, QLatin1Char('0')).toUpper();
                            ui->tableWidget_page2->item(i,2)->setText(value);
                        }
                        if(ui->tabWidget->currentIndex()==1)
                        {
                          onTabChanged(1);
                        }
                    }
                    else if(CMD_Buffer[CMDDATA]==BLOCK_ID_PAGE3)
                    {
                        for(int i=1;i<len-3;i++)
                        {
                            QString value="0x"+QString("%1").arg(CMD_Buffer[CMDDATA+1+i], 2, 16, QLatin1Char('0')).toUpper();
                            ui->tableWidget_page3->item(i,2)->setText(value);
                        }
                        if(ui->tabWidget->currentIndex()==2)
                        {
                          onTabChanged(2);
                        }
                    }
                    else if(CMD_Buffer[CMDDATA]==BLOCK_ID_PAGE4)
                    {
                        for(int i=1;i<len-3;i++)
                        {
                            QString value="0x"+QString("%1").arg(CMD_Buffer[CMDDATA+1+i], 2, 16, QLatin1Char('0')).toUpper();
                            ui->tableWidget_page4->item(i,2)->setText(value);
                        }
                        if(ui->tabWidget->currentIndex()==3)
                        {
                          onTabChanged(3);
                        }
                    }
                    else if(CMD_Buffer[CMDDATA]==BLOCK_ID_INFO1)
                    {
                        memcpy(&info,&CMD_Buffer[CMDDATA+1],sizeof(DeviceInfo));

                        m_deviceidCard->setDeviceId(QString::fromLatin1(info.systemSerialnumber));
                        //ui->deviceIDlabel->setText(QString::fromLatin1(info.systemSerialnumber));
                        //setWindowTitle(AppVersion+" Device ID:"+QString::fromLatin1(info.systemSerialnumber));
                        if(ui->tabWidget->currentIndex()==4)
                        {
                            onTabChanged(4);
                        }
                    }
                    else if(CMD_Buffer[CMDDATA]==BLOCK_ID_INFO2)
                    {
                        memcpy(&note,&CMD_Buffer[CMDDATA+1],sizeof(DeviceInfoNote));

                        if(ui->tabWidget->currentIndex()==4)
                        {
                            onTabChanged(4);
                        }
                    }
                }
                else
                {
                   qDebug() <<"[MainWindow]READ_EEPROM_BLOCK_RESPONSE_CMD checksum error!!";
                }
            }
            break;
            case READ_PCMD_BLOCK_RESPONSE_CMD:
            {
                uint8_t len=CMD_Buffer[PAYLOADLEN];
                uint8_t checksum=calculateChecksum(&CMD_Buffer[CMDDATA+1],len-3);

                qDebug() <<"[MainWindow]READ_PCMD_BLOCK_RESPONSE_CMD checksum:"<<checksum<<CMD_Buffer[len+1]<<len<<" Page Number:"<<CMD_Buffer[CMDDATA];

                if(checksum==CMD_Buffer[len+1])
                {
                    if(CMD_Buffer[CMDDATA]==0)
                    {
                        for(int i=0;i<len-3;i++)
                        {
                            QString value="0x"+QString("%1").arg(CMD_Buffer[CMDDATA+1+i], 2, 16, QLatin1Char('0')).toUpper();
                            ui->tableWidget_page0->item(i,2)->setText(value);
                        }
                        if(ui->tabWidget->currentIndex()==0)
                        {
                            onTabChanged(0);
                        }
                    }
                    else if(CMD_Buffer[CMDDATA]==1)
                    {

                    }
                    else if(CMD_Buffer[CMDDATA]==2)
                    {
                        for(int i=0;i<len-3;i++)
                        {
                            QString value="0x"+QString("%1").arg(CMD_Buffer[CMDDATA+1+i], 2, 16, QLatin1Char('0')).toUpper();
                            ui->tableWidget_page2->item(i,2)->setText(value);
                        }
                        if(ui->tabWidget->currentIndex()==1)
                        {
                            onTabChanged(1);
                        }
                    }
                    else if(CMD_Buffer[CMDDATA]==3)
                    {
                        for(int i=0;i<len-3;i++)
                        {
                            QString value="0x"+QString("%1").arg(CMD_Buffer[CMDDATA+1+i], 2, 16, QLatin1Char('0')).toUpper();
                            ui->tableWidget_page3->item(i,2)->setText(value);
                        }

                        if(ui->tabWidget->currentIndex()==2)
                        {
                            onTabChanged(2);
                        }
                    }
                    else if(CMD_Buffer[CMDDATA]==4)
                    {
                        for(int i=0;i<len-3;i++)
                        {
                            QString value="0x"+QString("%1").arg(CMD_Buffer[CMDDATA+1+i], 2, 16, QLatin1Char('0')).toUpper();
                            ui->tableWidget_page4->item(i,2)->setText(value);
                        }
                        if(ui->tabWidget->currentIndex()==3)
                        {
                            onTabChanged(3);
                        }
                    }
                }
                else
                {
                   qDebug() <<"[MainWindow]READ_PCMD_BLOCK_RESPONSE_CMD checksum error!!";
                }
            }
            break;
            case 0xB0:

                if(!bUpgrade||!file.exists())
                    return ret;

                if (CMD_Buffer[CMDDATA] == 0x01)
                {
                    GenCommand(0xB1, NULL, 0);
                }
                else if (CMD_Buffer[CMDDATA] == 0x02)
                {
                    uint8_t Buffer[9];

                    Buffer[0] = (FileSize & 0xFF000000) >> 24;
                    Buffer[1] = (FileSize & 0x00FF0000) >> 16;
                    Buffer[2] = (FileSize & 0x0000FF00) >> 8;
                    Buffer[3] = (FileSize & 0x000000FF);

                    Buffer[4] = ((APP_StartAddress + flash_base_address) & 0xFF000000) >> 24;
                    Buffer[5] = ((APP_StartAddress + flash_base_address) & 0x00FF0000) >> 16;
                    Buffer[6] = ((APP_StartAddress + flash_base_address) & 0x0000FF00) >> 8;
                    Buffer[7] = ((APP_StartAddress + flash_base_address) & 0x000000FF);

                    Buffer[8] = DEV_ID_NUMBER;

                    GenCommand(OTA_UPGRADE_INIT_CMD, Buffer, 9);
                }
                else
                {
                    qDebug() <<"[MainWindow]Invalid device!!";
                    QMessageBox::information(NULL, "MessageBox", "Invalid device!!");
                    bUpgrade=false;
                    ui->upgradeBtn->setEnabled(true);
                    ui->tabWidget->setTabEnabled(0,true);
                    ui->tabWidget->setTabEnabled(1,true);
                    ui->tabWidget->setTabEnabled(2,true);
                    ui->tabWidget->setTabEnabled(3,true);
                    ui->tabWidget->setTabEnabled(4,true);
                }

                break;
            case DEBUG_MSG_RESPONE_CMD:
            {
                char buf[256]={0};

                memcpy(buf,&CMD_Buffer[CMDDATA],CMD_Buffer[PAYLOADLEN]-1);

                QString msg = "[eMIC FW]"+QString::fromLatin1(buf);

                //qDebug() << msg.toUtf8().constData();

                LogManager::instance()->log(LogLevel::FWdebug, msg.toUtf8().constData());
            }
            break;
            default:
                // TODO...
                // handling unsupported or invalid Lingo ID
                qDebug() << "[MainWindow]handling unsupported or invalid Lingo ID:" << CMD_Buffer[CMDID];
                break;
        }
    }

    return ret;
}

bool MainWindow::IsBufferHasData()
{
    if (DATA_BUF_INDEX != TEMP_DATA_BUF_INDEX)
    {
        return true;
    }
    return false;
}

char MainWindow::ReadReceivePacket()
{
    char ReturnValue = 0;

    if (IsBufferHasData())
    {

        ReturnValue = ReceivePacket[TEMP_DATA_BUF_INDEX];

        TEMP_DATA_BUF_INDEX += 1;

        if (TEMP_DATA_BUF_INDEX >= 1024)
            TEMP_DATA_BUF_INDEX = 0;
    }

    return ReturnValue;

}


uint8_t MainWindow::PreparePacketToParse()
{
    uint8_t result = CMD_FAIL;

    while (IsBufferHasData())
    {
        CMD_Buffer[CMD_INDEX] = ReadReceivePacket();
        //qDebug() <<"-%x-"<<CMD_Buffer[CMD_INDEX];
        if (CMD_Buffer[STARTBYTE] == hBYTE)
        {

            if(CMD_Buffer[PAYLOADLEN]>=RCV_CMD_MAX_LEN&&CMD_INDEX>0)
            {
                CMD_INDEX=0;
                memset(CMD_Buffer, 0x00, 256);
                break;
            }

            if (((uint8_t)CMD_Buffer[PAYLOADLEN]) == CMD_INDEX - 1)
            {
                CMD_INDEX = 0;
                result = CMD_SUCCESS;
                return result;
            }

            CMD_INDEX += 1;

        }
        else
        {
            if(CMD_INDEX!=0)
            {
                CMD_INDEX=0;
                memset(CMD_Buffer, 0x00, 256);
            }

        }

    }

    return result;
}

void MainWindow::ProcessPacket()
{
    if (PreparePacketToParse() == CMD_SUCCESS)
    {
        HandlePacket();

        if(IsBufferHasData())
            workerThread->wake();

        memset(CMD_Buffer, 0x00, 256);
    }
}


MainWindow::~MainWindow()
{
    qDebug() <<"[MainWindow]Close!!";
    if (workerThread) {
        workerThread->stop();
        workerThread->wait();
        delete workerThread;
    }
    comPortDisconnect();
    LogManager::instance()->closeLog();
    delete ui;


}
