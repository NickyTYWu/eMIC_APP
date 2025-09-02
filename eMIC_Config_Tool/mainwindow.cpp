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


//#define PID 0xDD
//#define VID 0x4D8

#define FILE_APP_VERSION_INFO 0xF900

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->setFixedSize(1300,700);
    ui->setupUi(this);

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

    clearBtn= nullptr;
    infoNoteEdit= nullptr;
    getTimeBtn= nullptr;
    writeConfig= nullptr;
    comboBox = nullptr;
    slider = nullptr;
    for(int i=0;i<8;i++)
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



    initInfoRadioButton();
    initPage0();
    initPage2();
    initPage3();
    initPage4();

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
                                         "  gridline-color: #FFFFFF;"
                                         "  border: 1px solid white;"
                                         "}"
                                         "QTableWidget::item {"
                                         "  padding: 1px;"
                                         "  background-color: #000000;"
                                         "  border: 1px solid #FFFFFF;"
                                         "}"
                                         "QTableWidget::item:selected {"
                                         "  background-color: #007ae6;"
                                         "}");


    ui->tableWidget_page2->setAlternatingRowColors(true);
    ui->tableWidget_page2->setColumnWidth(0,80);
    ui->tableWidget_page2->setColumnWidth(1,130);
    ui->tableWidget_page2->setStyleSheet("QTableWidget {"
                                         "  gridline-color: #FFFFFF;"
                                         "  border: 1px solid white;"
                                         "}"
                                         "QTableWidget::item {"
                                         "  padding: 1px;"
                                         "  background-color: #000000;"
                                         "  border: 1px solid #FFFFFF;"
                                         "}"
                                         "QTableWidget::item:selected {"
                                         "  background-color: #007ae6;"
                                         "}");


    ui->tableWidget_page3->setAlternatingRowColors(true);
    ui->tableWidget_page3->setColumnWidth(0,80);
    ui->tableWidget_page3->setColumnWidth(1,130);
    ui->tableWidget_page3->setStyleSheet("QTableWidget {"
                                         "  gridline-color: #FFFFFF;"
                                         "  border: 1px solid white;"
                                         "}"
                                         "QTableWidget::item {"
                                         "  padding: 1px;"
                                         "  background-color: #000000;"
                                         "  border: 1px solid #FFFFFF;"
                                         "}"
                                         "QTableWidget::item:selected {"
                                         "  background-color: #007ae6;"
                                         "}");



    ui->tableWidget_page4->setAlternatingRowColors(true);
    ui->tableWidget_page4->setColumnWidth(0,80);
    ui->tableWidget_page4->setColumnWidth(1,130);
    ui->tableWidget_page4->setStyleSheet("QTableWidget {"
                                         "  gridline-color: #FFFFFF;"
                                         "  border: 1px solid white;"
                                         "}"
                                         "QTableWidget::item {"
                                         "  padding: 1px;"
                                         "  background-color: #000000;"
                                         "  border: 1px solid #FFFFFF;"
                                         "}"
                                         "QTableWidget::item:selected {"
                                         "  background-color: #007ae6;"
                                         "}");

    ui->page2WriteRegBtn->setVisible(false);
    ui->page3WriteRegBtn->setVisible(false);
    ui->page4WriteRegBtn->setVisible(false);
    //connect(mSerialScanTimer, &QTimer::timeout,this, &MainWindow::updateSerialPorts);
    connect(mSerial, &QSerialPort::readyRead,this, &MainWindow::serialReadyRead);
    connect(ui->uartCH_comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(uartChannelChange(int)));
    connect(ui->connectBtn,SIGNAL(clicked()),this,SLOT(connectBtn_clicked()));
    connect(ui->disconnectBtn,SIGNAL(clicked()),this,SLOT(disconnectBtn_clicked()));
    connect(ui->selectFileBtn,SIGNAL(clicked()),this,SLOT(openFileBtn_clicked()));

    connect(ui->upgradeBtn,SIGNAL(clicked()),this,SLOT(upgradeBtn_clicked()));
    connect(ui->page0RefreshBtn,SIGNAL(clicked()),this,SLOT(page0RefreshBtn_clicked()));
    connect(ui->page0SaveBtn,SIGNAL(clicked()),this,SLOT(page0SaveBtn_clicked()));
    connect(ui->page0ImportBtn,SIGNAL(clicked()),this,SLOT(page0ImportBtn_clicked()));
    connect(ui->page0ExportBtn,SIGNAL(clicked()),this,SLOT(page0ExportBtn_clicked()));
    connect(ui->page0WriteRegBtn,SIGNAL(clicked()),this,SLOT(page0WriteRegBtn_clicked()));
    connect(ui->page0WriteAllBtn,SIGNAL(clicked()),this,SLOT(page0WriteAllBtn_clicked()));

    connect(ui->page2RefreshBtn,SIGNAL(clicked()),this,SLOT(page2RefreshBtn_clicked()));
    connect(ui->page2SaveBtn,SIGNAL(clicked()),this,SLOT(page2SaveBtn_clicked()));
    connect(ui->page2ImportBtn,SIGNAL(clicked()),this,SLOT(page2ImportBtn_clicked()));
    connect(ui->page2ExportBtn,SIGNAL(clicked()),this,SLOT(page2ExportBtn_clicked()));
    connect(ui->page2WriteRegBtn,SIGNAL(clicked()),this,SLOT(page2WriteRegBtn_clicked()));
    connect(ui->page2WriteAllBtn,SIGNAL(clicked()),this,SLOT(page2WriteAllBtn_clicked()));

    connect(ui->page3RefreshBtn,SIGNAL(clicked()),this,SLOT(page3RefreshBtn_clicked()));
    connect(ui->page3SaveBtn,SIGNAL(clicked()),this,SLOT(page3SaveBtn_clicked()));
    connect(ui->page3ImportBtn,SIGNAL(clicked()),this,SLOT(page3ImportBtn_clicked()));
    connect(ui->page3ExportBtn,SIGNAL(clicked()),this,SLOT(page3ExportBtn_clicked()));
    connect(ui->page3WriteRegBtn,SIGNAL(clicked()),this,SLOT(page3WriteRegBtn_clicked()));
    connect(ui->page3WriteAllBtn,SIGNAL(clicked()),this,SLOT(page3WriteAllBtn_clicked()));

    connect(ui->page4RefreshBtn,SIGNAL(clicked()),this,SLOT(page4RefreshBtn_clicked()));
    connect(ui->page4SaveBtn,SIGNAL(clicked()),this,SLOT(page4SaveBtn_clicked()));
    connect(ui->page4ImportBtn,SIGNAL(clicked()),this,SLOT(page4ImportBtn_clicked()));
    connect(ui->page4ExportBtn,SIGNAL(clicked()),this,SLOT(page4ExportBtn_clicked()));
    connect(ui->page4WriteRegBtn,SIGNAL(clicked()),this,SLOT(page4WriteRegBtn_clicked()));
    connect(ui->page4WriteAllBtn,SIGNAL(clicked()),this,SLOT(page4WriteAllBtn_clicked()));

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(ui->tableWidget_page0->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(tableWidgetPage0SelectChange(QItemSelection,QItemSelection)));
    connect(ui->tableWidget_page2->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(tableWidgetPage2SelectChange(QItemSelection,QItemSelection)));
    connect(ui->tableWidget_page3->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(tableWidgetPage3SelectChange(QItemSelection,QItemSelection)));
    connect(ui->tableWidget_page4->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(tableWidgetPage4SelectChange(QItemSelection,QItemSelection)));

    connect(workerThread, &CheckWorkerThread::checkFunctionCalled, this, &MainWindow::onCheckFunctionCalled);
    connect(workerThread, &CheckWorkerThread::workFinished, this, &MainWindow::onWorkFinished);

    updateSerialPorts();

    workerThread->start();
    mSerialScanTimer.start();

    ui->tableWidget_page4->setCurrentCell(1,0);
    ui->tableWidget_page3->setCurrentCell(1,0);
    ui->tableWidget_page2->setCurrentCell(1,0);
    ui->tableWidget_page0->setCurrentCell(1,0);
}

void MainWindow::initInfoRadioButton()
{
    qDebug() << "[MainWindow]initInfoRadioButton";
    QWidget* tabPage = ui->tabWidget->widget(4);

    infoBtnGrup = new QButtonGroup(tabPage);

    infoGroupBox = new QGroupBox(tabPage);
    infoGroupBox->setStyleSheet(QStringLiteral("QGroupBox{border:1px solid white;font-weight: bold;font-size: 10pt;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top left;padding:0.3px;}"));

    for(int i=0;i<4;i++)
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

    //infoRadioButton[4]->setGeometry(40,180,240,20);
    //infoRadioButton[4]->setText("Show Reference Frequency Response");
    //infoRadioButton[4]->setEnabled(false);
    //infoRadioButton[4]->show();

    //infoRadioButton[5]->setGeometry(40,210,240,20);
    //infoRadioButton[5]->setText("Edit Reference Frequency Response");
    //infoRadioButton[5]->setEnabled(false);
    //infoRadioButton[5]->show();


    for(int i=0;i<4;i++)
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

    GenCommand(GET_FW_VERSION_CMD, NULL, 0);
    GenCommand(GET_TH_CMD, NULL, 0);
    refreshInfo();
    refreshNote();

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
    mSerial->flush();

    syncDeviceData();
}

void MainWindow::comPortDisconnect()
{
    qDebug() << "[MainWindow]comPortDisconnect()";

    IsConnect=false;
    Mcp2221_CloseAll();
    handle = NULL;
    if (mSerial->isOpen()) {
        qDebug() << "Serial already connected, disconnecting!";
        mSerial->close();
    }
    ui->temperatureLabel->setText("Temperature:");
    ui->humidtyLabel->setText("Humidty:");
    ui->deviceIDlabel->setText("Device ID:");
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

void MainWindow::page0SaveBtn_clicked()
{
    uint8_t pageBuf[PAGE0_REG_MAX_SIZE+2];

    qDebug() << "[MainWindow]page0SaveBtn_clicked()";

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

void MainWindow::page2SaveBtn_clicked()
{
    uint8_t pageBuf[PAGE2_REG_MAX_SIZE+2];

    qDebug() << "[MainWindow]page2SaveBtn_clicked()";

    pageBuf[0]=BLOCK_ID_PAGE2;

    for(int i=1;i<PAGE2_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page2->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE2_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE2_REG_MAX_SIZE);
    GenCommand(WRITE_EEPROM_BLOCK_CMD, pageBuf,PAGE2_REG_MAX_SIZE+2);
}

void MainWindow::page3SaveBtn_clicked()
{
    uint8_t pageBuf[PAGE3_REG_MAX_SIZE+2];

    qDebug() << "[MainWindow]page3SaveBtn_clicked()";

    pageBuf[0]=BLOCK_ID_PAGE3;

    for(int i=1;i<PAGE3_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page3->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE3_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE3_REG_MAX_SIZE);
    GenCommand(WRITE_EEPROM_BLOCK_CMD, pageBuf,PAGE3_REG_MAX_SIZE+2);
}

void MainWindow::page4SaveBtn_clicked()
{
    uint8_t pageBuf[PAGE4_REG_MAX_SIZE+2];

    qDebug() << "[MainWindow]page4SaveBtn_clicked()";

    pageBuf[0]=BLOCK_ID_PAGE4;

    for(int i=1;i<PAGE4_REG_MAX_SIZE+1;i++)
    {
        pageBuf[i]=(ui->tableWidget_page4->item(i-1,2)->text()).toInt(NULL,16);
    }

    pageBuf[PAGE4_REG_MAX_SIZE+1]=calculateChecksum(&pageBuf[1],PAGE4_REG_MAX_SIZE);
    GenCommand(WRITE_EEPROM_BLOCK_CMD, pageBuf,PAGE4_REG_MAX_SIZE+2);
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

void MainWindow::regSlelectChange(int index)
{
    qDebug() << "[MainWindow]regSlelectChange() index:"<<index;

    if(ui->tabWidget->currentIndex()==0)
    {
        uint8_t RegValue=(ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->text()).toInt(NULL,16);
        if(ui->tableWidget_page0->currentRow()==4)
        {
            RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup3Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup3Offset);
            RegValue|=index<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup3Offset;
        }
        else
        {
            RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset);
            RegValue|=index<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset;
        }
        QString value="0x"+QString("%1").arg(RegValue, 2, 16, QLatin1Char('0')).toUpper();
        ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText(value);

        if(ui->tableWidget_page0->currentRow()==4)
        {
            if(comboBox->currentIndex()==0)
                descriptionLabel[3]->setText("ASI data MSB location has no offset and is as per standard protocol");
            else
            {

                descriptionLabel[3]->setText(QString::asprintf("ASI data MSB location (TDM mode is slot 0 or I2S, LJ mode is the left and right slot 0)\r\noffset of %d BCLK cycle with respect to standard protocol",comboBox->currentIndex()));
            }
        }
        else
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
    RegValue=RegValue&~(regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Mask<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset);
    RegValue|=index<<regOffset[ui->tableWidget_page0->currentRow()].btnGrup1Offset;


    ui->tableWidget_page0->item(ui->tableWidget_page0->currentRow(),2)->setText(QString("0x%1").arg(RegValue, 2, 16, QLatin1Char('0')));

    if(ui->tableWidget_page0->currentRow()==27||ui->tableWidget_page0->currentRow()==31||ui->tableWidget_page0->currentRow()==34||ui->tableWidget_page0->currentRow()==37)
    {
        if(index==0)
            descriptionLabel[1]->setText(QString::asprintf("Digital volume is muted"));
        else
        {

            descriptionLabel[1]->setText(QString::asprintf("Digital volume control is set to %.1f dB",((index-1)*0.5)-100));
        }
    }
    else if(ui->tableWidget_page0->currentRow()==29||ui->tableWidget_page0->currentRow()==33||ui->tableWidget_page0->currentRow()==36||ui->tableWidget_page0->currentRow()==39)
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

    for(int i=0;i<8;i++)
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
            else if(memberIndex==46)
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
                if(ui->tableWidget_page0->currentRow()==4)
                {
                    if(comboBox->currentIndex()==0)
                        descriptionLabel[group+1]->setText("ASI data MSB location has no offset and is as per standard protocol");
                    else
                    {

                        descriptionLabel[group+1]->setText(QString::asprintf("ASI data MSB location (TDM mode is slot 0 or I2S, LJ mode is the left and right slot 0)\r\noffset of %d BCLK cycle with respect to standard protocol",comboBox->currentIndex()));
                    }
                }
                else
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

            slider = new QSlider(Qt::Horizontal,this);
            slider->setRange(Slider_MinValue,Slider_MaxValue);

            slider->setGeometry(Slider_X,Slider_Y,Slider_W,Slider_H);
            slider->setSliderPosition(checkRegValue);
            slider->setTickInterval(10);
            slider->setTickPosition(QSlider::TicksBelow);
            slider->show();

            descriptionLabel[group+1] = new QLabel(this);
            descriptionLabel[group+1]->setGeometry(Result_Description_X,Result_Description_Y,Result_Description_W,Result_Description_H);
            if(ui->tableWidget_page0->currentRow()==27||ui->tableWidget_page0->currentRow()==31||ui->tableWidget_page0->currentRow()==34||ui->tableWidget_page0->currentRow()==37)
            {
                if(checkRegValue==0)
                    descriptionLabel[group+1]->setText(QString::asprintf("Digital volume is muted"));
                else
                {

                    descriptionLabel[group+1]->setText(QString::asprintf("Digital volume control is set to %.1f dB",((checkRegValue-1)*0.5)-100));
                }
            }
            else if(ui->tableWidget_page0->currentRow()==29||ui->tableWidget_page0->currentRow()==33||ui->tableWidget_page0->currentRow()==36||ui->tableWidget_page0->currentRow()==39)
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
    //infoLineEdit[0]->setText("IM72D128");
    infoLineEdit[0]->show();

    descriptionLabel[1]->setGeometry(460,140,385,20);
    descriptionLabel[1]->setText("Cartridge 1 Version Letter:");
    descriptionLabel[1]->show();

    infoLineEdit[1]->setGeometry(660,140,130,20);
    infoLineEdit[1]->setMaxLength(1);
    infoLineEdit[1]->setText(QString::fromLatin1(info.cartridge1VersionLetter));
    //infoLineEdit[1]->setText("A");
    infoLineEdit[1]->show();

    descriptionLabel[2]->setGeometry(460,165,385,20);
    descriptionLabel[2]->setText("Cartridge 1 Serial Number:");
    descriptionLabel[2]->show();

    infoLineEdit[2]->setGeometry(660,165,130,20);
    infoLineEdit[2]->setMaxLength(16);
    infoLineEdit[2]->setText(QString::fromLatin1(info.cartridge1SerialNumber));
    //infoLineEdit[2]->setText("SN20250828A00001");
    infoLineEdit[2]->show();

    descriptionLabel[3]->setGeometry(460,190,385,20);
    descriptionLabel[3]->setText("Cartridge 1 Sensitivity:");
    descriptionLabel[3]->show();

    infoLineEdit[3]->setGeometry(660,190,130,20);
    infoLineEdit[3]->setValidator(validator);
    infoLineEdit[3]->setText(QString::number(info.cartridge1Sensitivity,'f',2));
    //infoLineEdit[3]->setText("50.1");
    infoLineEdit[3]->show();

    descriptionLabel[4]->setGeometry(460,215,385,20);
    descriptionLabel[4]->setText("Cartridge 1 Reference Frequency:");
    descriptionLabel[4]->show();

    infoLineEdit[4]->setGeometry(660,215,130,20);
    infoLineEdit[4]->setValidator(validator);
    infoLineEdit[4]->setText(QString::number(info.cartridge1ReferenceFrequency,'f',2));
    //infoLineEdit[4]->setText("1000");
    infoLineEdit[4]->show();

    descriptionLabel[5]->setGeometry(460,240,385,20);
    descriptionLabel[5]->setText("Cartridge 1 Units Code:");
    descriptionLabel[5]->show();

    infoLineEdit[5]->setGeometry(660,240,130,20);
    infoLineEdit[5]->setValidator(byteValidator);
    infoLineEdit[5]->setText(QString::number(info.cartridge1UnitsCode));
    //infoLineEdit[5]->setText("2");
    infoLineEdit[5]->show();

    descriptionLabel[6]->setGeometry(460,265,385,20);
    descriptionLabel[6]->setText("Cartridge 1 Frequency Range Min:");
    descriptionLabel[6]->show();

    infoLineEdit[6]->setGeometry(660,265,130,20);
    infoLineEdit[6]->setValidator(intValidator);
    infoLineEdit[6]->setText(QString::number(info.cartridge1FrequencyRangeMin));
    //infoLineEdit[6]->setText("10");
    infoLineEdit[6]->show();

    descriptionLabel[7]->setGeometry(460,290,385,20);
    descriptionLabel[7]->setText("Cartridge 1 Frequency Range Max:");
    descriptionLabel[7]->show();

    infoLineEdit[7]->setGeometry(660,290,130,20);
    infoLineEdit[7]->setValidator(intValidator);
    infoLineEdit[7]->setText(QString::number(info.cartridge1FrequencyRangeMax));
    //infoLineEdit[7]->setText("2000");
    infoLineEdit[7]->show();

    descriptionLabel[8]->setGeometry(460,315,385,20);
    descriptionLabel[8]->setText("Cartridge 1 Channel Assignment:");
    descriptionLabel[8]->show();

    infoLineEdit[8]->setGeometry(660,315,130,20);
    infoLineEdit[8]->setValidator(byteValidator);
    infoLineEdit[8]->setText(QString::number(info.cartridge1ChannelAssignment));
    //infoLineEdit[8]->setText("1");
    infoLineEdit[8]->show();

    descriptionLabel[9]->setGeometry(800,115,385,20);
    descriptionLabel[9]->setText("Cartridge 2 Model Number:");
    descriptionLabel[9]->show();

    infoLineEdit[9]->setGeometry(1000,115,130,20);
    infoLineEdit[9]->setMaxLength(8);
    infoLineEdit[9]->setText(QString::fromLatin1(info.cartridge2ModelNumber));
    //infoLineEdit[9]->setText("IM63D135");
    infoLineEdit[9]->show();

    descriptionLabel[10]->setGeometry(800,140,385,20);
    descriptionLabel[10]->setText("Cartridge 2 Version Letter:");
    descriptionLabel[10]->show();

    infoLineEdit[10]->setGeometry(1000,140,130,20);
    infoLineEdit[10]->setMaxLength(1);
    infoLineEdit[10]->setText(QString::fromLatin1(info.cartridge2VersionLetter));
    //infoLineEdit[10]->setText("A");
    infoLineEdit[10]->show();

    descriptionLabel[11]->setGeometry(800,165,385,20);
    descriptionLabel[11]->setText("Cartridge 2 Serial Number:");
    descriptionLabel[11]->show();

    infoLineEdit[11]->setGeometry(1000,165,130,20);
    infoLineEdit[1]->setMaxLength(16);
    infoLineEdit[11]->setText(QString::fromLatin1(info.cartridge2SerialNumber));
    //infoLineEdit[11]->setText("SN20250828B00001");
    infoLineEdit[11]->show();

    descriptionLabel[12]->setGeometry(800,190,385,20);
    descriptionLabel[12]->setText("Cartridge 2 Sensitivity:");
    descriptionLabel[12]->show();

    infoLineEdit[12]->setGeometry(1000,190,130,20);
    infoLineEdit[12]->setValidator(validator);
    infoLineEdit[12]->setText(QString::number(info.cartridge2Sensitivity,'f',2));
    //infoLineEdit[12]->setText("50.1");
    infoLineEdit[12]->show();

    descriptionLabel[13]->setGeometry(800,215,385,20);
    descriptionLabel[13]->setText("Cartridge 2 Reference Frequency:");
    descriptionLabel[13]->show();

    infoLineEdit[13]->setGeometry(1000,215,130,20);
    infoLineEdit[13]->setValidator(validator);
    infoLineEdit[13]->setText(QString::number(info.cartridge2ReferenceFrequency,'f',2));
    //infoLineEdit[13]->setText("1000");
    infoLineEdit[13]->show();

    descriptionLabel[14]->setGeometry(800,240,385,20);
    descriptionLabel[14]->setText("Cartridge 2 Units Code:");
    descriptionLabel[14]->show();

    infoLineEdit[14]->setGeometry(1000,240,130,20);
    infoLineEdit[14]->setValidator(byteValidator);
    infoLineEdit[14]->setText(QString::number(info.cartridge2UnitsCode));
    //infoLineEdit[14]->setText("2");
    infoLineEdit[14]->show();

    descriptionLabel[15]->setGeometry(800,265,385,20);
    descriptionLabel[15]->setText("Cartridge 2 Frequency Range Min:");
    descriptionLabel[15]->show();

    infoLineEdit[15]->setGeometry(1000,265,130,20);
    infoLineEdit[15]->setValidator(intValidator);
    infoLineEdit[15]->setText(QString::number(info.cartridge2FrequencyRangeMin));
    //infoLineEdit[15]->setText("10");
    infoLineEdit[15]->show();

    descriptionLabel[16]->setGeometry(800,290,385,20);
    descriptionLabel[16]->setText("Cartridge 2 Frequency Range Max:");
    descriptionLabel[16]->show();

    infoLineEdit[16]->setGeometry(1000,290,130,20);
    infoLineEdit[16]->setValidator(intValidator);
    infoLineEdit[16]->setText(QString::number(info.cartridge2FrequencyRangeMax));
    //infoLineEdit[16]->setText("2000");
    infoLineEdit[16]->show();

    descriptionLabel[17]->setGeometry(800,315,385,20);
    descriptionLabel[17]->setText("Cartridge 2 Channel Assignment:");
    descriptionLabel[17]->show();

    infoLineEdit[17]->setGeometry(1000,315,130,20);
    infoLineEdit[17]->setValidator(byteValidator);
    infoLineEdit[17]->setText(QString::number(info.cartridge2ChannelAssignment));
    //infoLineEdit[17]->setText("2");
    infoLineEdit[17]->show();

    descriptionLabel[18]->setGeometry(460,365,385,20);
    descriptionLabel[18]->setText("System Digital Interface Type:");
    descriptionLabel[18]->show();

    infoLineEdit[18]->setGeometry(660,365,130,20);
    infoLineEdit[18]->setMaxLength(4);
    infoLineEdit[18]->setText(QString::fromLatin1(info.systemDigitInterfaceType));
    //infoLineEdit[18]->setText("I2S");
    infoLineEdit[18]->show();

    descriptionLabel[19]->setGeometry(460,390,385,20);
    descriptionLabel[19]->setText("System Bit Clock Frequency:");
    descriptionLabel[19]->show();

    infoLineEdit[19]->setGeometry(660,390,130,20);
    infoLineEdit[19]->setValidator(validator);
    infoLineEdit[19]->setText(QString::number(info.systemBitClockFrequency,'f',4));
    //infoLineEdit[19]->setText("3.072");
    infoLineEdit[19]->show();

    descriptionLabel[20]->setGeometry(460,415,385,20);
    descriptionLabel[20]->setText("System Word Length:");
    descriptionLabel[20]->show();

    infoLineEdit[20]->setGeometry(660,415,130,20);
    infoLineEdit[20]->setValidator(byteValidator);
    infoLineEdit[20]->setText(QString::number(info.systemWordLength));
    //infoLineEdit[20]->setText("24");
    infoLineEdit[20]->show();

    descriptionLabel[21]->setGeometry(460,440,385,20);
    descriptionLabel[21]->setText("System Sample Rate:");
    descriptionLabel[21]->show();

    infoLineEdit[21]->setGeometry(660,440,130,20);
    infoLineEdit[21]->setValidator(validator);
    infoLineEdit[21]->setText(QString::number(info.systemSampleRate,'f',3));
    //infoLineEdit[21]->setText("48000");
    infoLineEdit[21]->show();

    descriptionLabel[22]->setGeometry(460,465,385,20);
    descriptionLabel[22]->setText("System Serial Number:");
    descriptionLabel[22]->show();

    infoLineEdit[22]->setGeometry(660,465,130,20);
    infoLineEdit[22]->setMaxLength(16);
    infoLineEdit[22]->setText(QString::fromLatin1(info.systemSerialnumber));
    //infoLineEdit[22]->setText("SN20250828AB0001");
    infoLineEdit[22]->show();

    descriptionLabel[23]->setGeometry(460,490,385,20);
    descriptionLabel[23]->setText("System Sensitivity:");
    descriptionLabel[23]->show();

    infoLineEdit[23]->setGeometry(660,490,130,20);
    infoLineEdit[23]->setValidator(validator);
    infoLineEdit[23]->setText(QString::number(info.systemSensitivity,'f',2));
    //infoLineEdit[23]->setText("-50");
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
    //infoLineEdit[25]->setText("0x0000");
    infoLineEdit[25]->show();

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

    if(!bUpgrade)
    {
        if(IsConnect)
            bCheckDeviceOffLine=true;
        GenCommand(GET_TH_CMD, NULL, 0);
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
        if(IsConnect)
        {
            if(bCheckDeviceOffLine)
            {
                if(!bReSyncDataWhenDeviceOnLine)
                {
                    bReSyncDataWhenDeviceOnLine=true;
                    ui->temperatureLabel->setText("Temperature:");
                    ui->humidtyLabel->setText("Humidty:");
                    ui->deviceIDlabel->setText("Device ID:");
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
        }

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
                        qDebug() <<"[MainWindow]Save Checksum Fail!!";
                        QMessageBox::information(NULL, "MessageBox", "Save Checksum Fail!!");

                    }
                    else if (CMD_Buffer[CMDDATA + 1] == CMD_PAGE0_WRITE_SUCCESS
                           ||CMD_Buffer[CMDDATA + 1] == CMD_PAGE2_WRITE_SSUCCESS
                           ||CMD_Buffer[CMDDATA + 1] == CMD_PAGE3_WRITE_SSUCCESS
                           ||CMD_Buffer[CMDDATA + 1] == CMD_PAGE4_WRITE_SSUCCESS)
                    {
                        GenCommand(0xAA, NULL, 0);
                        qDebug() <<"[MainWindow]Save Success!!";
                        QMessageBox::information(NULL, "MessageBox", "Save Success!!");

                    }
                    else if (CMD_Buffer[CMDDATA + 1] == CMD_INFO1_WRITE_SSUCCESS)
                    {
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
                }

            break;
            case GET_FW_VERSION_RESPONSE_CMD:
            {
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
                qDebug() <<"[MainWindow]GET_FW_VERSION_RESPONSE_CMD Version::"<<version;
            }
            break;
            case GET_TH_RESPONSE_CMD:
            {

                uint32_t temp=CMD_Buffer[CMDDATA]<<24|CMD_Buffer[CMDDATA+1]<<16|CMD_Buffer[CMDDATA+2]<<8|CMD_Buffer[CMDDATA+3];
                uint32_t humidity=CMD_Buffer[CMDDATA+4]<<24|CMD_Buffer[CMDDATA+5]<<16|CMD_Buffer[CMDDATA+6]<<8|CMD_Buffer[CMDDATA+7];

                ui->temperatureLabel->setText("Temperature:"+QLocale().toString(temp/1000.0));
                ui->humidtyLabel->setText("Humidty:"+QLocale().toString(humidity/1000.0));
                qDebug() <<"[MainWindow]GET_TH_RESPONSE_CMD temperatureLabel:"<<temp;
                qDebug() <<"[MainWindow]GET_TH_RESPONSE_CMD humidtyLabel:"<<humidity;
            }
            break;
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

                        ui->deviceIDlabel->setText("Device ID: "+QString::fromLatin1(info.systemSerialnumber));

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
                    uint8_t Buffer[8];

                    Buffer[0] = (FileSize & 0xFF000000) >> 24;
                    Buffer[1] = (FileSize & 0x00FF0000) >> 16;
                    Buffer[2] = (FileSize & 0x0000FF00) >> 8;
                    Buffer[3] = (FileSize & 0x000000FF);

                    Buffer[4] = ((APP_StartAddress + flash_base_address) & 0xFF000000) >> 24;
                    Buffer[5] = ((APP_StartAddress + flash_base_address) & 0x00FF0000) >> 16;
                    Buffer[6] = ((APP_StartAddress + flash_base_address) & 0x0000FF00) >> 8;
                    Buffer[7] = ((APP_StartAddress + flash_base_address) & 0x000000FF);

                    GenCommand(OTA_UPGRADE_INIT_CMD, Buffer, 8);
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
        //qDebug() <<"%x"<<CMD_Buffer[CMD_INDEX];
        if (CMD_Buffer[STARTBYTE] == hBYTE)
        {

            if(CMD_Buffer[PAYLOADLEN]>=RCV_CMD_MAX_LEN)
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
