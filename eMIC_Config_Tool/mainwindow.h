#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QTableView>
#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QTableWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include "page0datamanager.h"
#include "page2datamanager.h"
#include "page3datamanager.h"
#include "page4datamanager.h"
#include "DeviceInfo.h"
#include "CheckWorkerThread.h"
#include <QElapsedTimer>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void uartChannelChange(int index);
    void serialReadyRead();
    void connectBtn_clicked();
    void disconnectBtn_clicked();
    void openFileBtn_clicked();
    void upgradeBtn_clicked();
    void page0RefreshBtn_clicked();
    void page0SaveBtn_clicked();
    void page0ImportBtn_clicked();
    void page0ExportBtn_clicked();
    void page0WriteRegBtn_clicked();
    void page0WriteAllBtn_clicked();
    void page2RefreshBtn_clicked();
    void page2SaveBtn_clicked();
    void page2ImportBtn_clicked();
    void page2ExportBtn_clicked();
    void page2WriteRegBtn_clicked();
    void page2WriteAllBtn_clicked();
    void page3RefreshBtn_clicked();
    void page3SaveBtn_clicked();
    void page3ImportBtn_clicked();
    void page3ExportBtn_clicked();
    void page3WriteRegBtn_clicked();
    void page3WriteAllBtn_clicked();
    void page4RefreshBtn_clicked();
    void page4SaveBtn_clicked();
    void page4ImportBtn_clicked();
    void page4ExportBtn_clicked();
    void page4WriteRegBtn_clicked();
    void page4WriteAllBtn_clicked();
    void getTimeBtn_clicked();
    void writeConfig_clicked();
    void clearBtn_clicked();
    void tableWidgetPage0SelectChange(QItemSelection selected,QItemSelection deselected);
    void tableWidgetPage2SelectChange(QItemSelection selected,QItemSelection deselected);
    void tableWidgetPage3SelectChange(QItemSelection selected,QItemSelection deselected);
    void tableWidgetPage4SelectChange(QItemSelection selected,QItemSelection deselected);
    void handleGroupClick(QAbstractButton* button);
    void infoRadiobtnGroup1clicked(QAbstractButton *button);
    void regSlelectChange(int index);
    void regSliderChange(int index);
    void onTabChanged(int index);
    void onCheckFunctionCalled(int count);
    void onWorkFinished();


private:
    Ui::MainWindow *ui;
    QSerialPort *mSerial;
    QList<QSerialPortInfo> mSerialPorts;
    QElapsedTimer mSerialScanTimer;
    void updateSerialPorts();

    bool checkMCP2221SerialNumberEnumerationEnable(unsigned int VID, unsigned int PID);
    bool initMCP2221(QString SerialNum,unsigned int VID, unsigned int PID);
    void *handle;
    wchar_t SerNum;
    wchar_t LibVer[6];
    wchar_t MfrDescriptor[30]={0};
    wchar_t ProdDescrip[30]={0};
    wchar_t PordSerNum[30]={0};
    unsigned int mcpVID=0,mcpPID=0;
    int ver;
    int error;
    int flag;
    unsigned int delay;
    unsigned int ReqCurrent;
    unsigned int NumOfDev;
    unsigned char PowerAttrib;
    uint8_t checkComPortIsMcp2221;
    QString Auto_Connect_Serial_Number;
    QString Auto_Connect_VID;
    QString Auto_Connect_PID;
    QString autoConnectPortName;
    bool bAutoConnect;
    uint8_t needRecordInfo;
    uint8_t needAutoRefreshWhenConnect;
    uint8_t autoRefreshSource;
    uint8_t FW_Debug_Msg;
    CheckWorkerThread* workerThread = nullptr;

    void updateUartChannel(int index);
    void ProcessPacket();
    uint8_t PreparePacketToParse();
    uint8_t HandlePacket();
    char ReadReceivePacket();
    uint8_t calculateChecksum(uint8_t *data,uint8_t len);
    void GenCommand(uint8_t nCMD, uint8_t *pCMD, uint16_t nCmdLen);
    int SendCMD(char* szMsg, uint16_t nCmdLen);
    bool IsBufferHasData();
    void comPortDisconnect();
    void comPortConnect();
    bool checkFileFormat();
    void syncDeviceData();
    uint32_t ImageCheckSum;
    QString fileName;
    QFile file;
    //QDataStream in;
    uint32_t FileSize;
    bool IsConnect, bUpgrade;
    uint16_t DATA_BUF_INDEX;
    uint16_t TEMP_DATA_BUF_INDEX;
    char ReceivePacket[1024];
    char g_cBsdBuf[1024];
    uint8_t CMD_Buffer[256];
    int CMD_INDEX;
    int   UpgradePos;
    short range;
    uint32_t imagesize;
    uint16_t version;
    QString imageVersion;
    bool bSend0xB0CommandAfterReset;
    bool bCheckDeviceOffLine;
    bool bReSyncDataWhenDeviceOnLine;

    bool bRefreshInfoBlock1;
    bool bRefreshInfoBlock2;
    QButtonGroup  *infoBtnGrup;
    QGroupBox *infoGroupBox;
    QPushButton *infoRefreshBtn;
    QRadioButton *infoRadioButton[6];
    int fromBCD(uint8_t bcd);
    uint8_t toBCD(int val);
    QDateTime bcdToDateTime(uint8_t *bcd);
    void dateTimeToBCD(uint8_t *bcd,const QDateTime& dt);

    void recordWriteInfo();
    void rotateInfoRecordFileIfNeeded();
    void recordWriteNote();
    void rotateNoteRecordFileIfNeeded();
    void recordWriteHubInfo();
    void rotateHubInfoRecordFileIfNeeded();

    QString m_logDir = "info_record";
    QString m_curDate;
    QFile m_file;
    QStringList oldValueList;
    bool bWriteRecordInfo;

    QPushButton *getTimeBtn;
    QPushButton *writeConfig;
    QPushButton *clearBtn;
    QTextEdit   *infoNoteEdit;
    QButtonGroup *btnGrup[8];
    QGroupBox *GroupBox[8];
    QLineEdit *infoLineEdit[40];
    QLabel *descriptionLabel[40];
    QRadioButton *radioButton[40];
    QComboBox *comboBox;
    QSlider *slider;

    void exportToCSV(QTableWidget *tableWidget, const QString &filePath);
    void importFromCSV(QTableWidget *tableWidget, const QString &filePath);
    bool checkImportCSVFormat(QTableWidget *tableWidget, const QString &filePath);
    bool isValidHexByte(const QString &str);
    QString normalizeHexByte(QString str);

    struct RegValueOffset{
        uint8_t rowIndex;
        uint8_t btnGrup1Offset;
        uint8_t btnGrup1Mask;
        uint8_t btnGrup2Offset;
        uint8_t btnGrup2Mask;
        uint8_t btnGrup3Offset;
        uint8_t btnGrup3Mask;
        uint8_t btnGrup4Offset;
        uint8_t btnGrup4Mask;
        uint8_t btnGrup5Offset;
        uint8_t btnGrup5Mask;
        uint8_t btnGrup6Offset;
        uint8_t btnGrup6Mask;
        uint8_t btnGrup7Offset;
        uint8_t btnGrup7Mask;
        uint8_t btnGrup8Offset;
        uint8_t btnGrup8Mask;
    };
    RegValueOffset  regOffset[48];
    RegValueOffset  pag2RegOffset[1];
    RegValueOffset  pag3RegOffset[1];
    RegValueOffset  pag4RegOffset[1];

    Page0DataManager page0Manager;
    Page2DataManager page2Manager;
    Page3DataManager page3Manager;
    Page4DataManager page4Manager;

    DeviceInfo  info;
    DeviceInfoNote note;
    void initPage0();
    void initPage2();
    void initPage3();
    void initPage4();

    void initInfoRadioButton();
    void refreshInfo();
    void refreshNote();
    void refreshPage(uint8_t pageID,uint8_t readFrom);
    void enableFWLog(uint8_t bEnable);

    void deletePage();
    void DrawPageComponent(const QString& filename, int memberIndex);

    void createShowInfo();
    void createEditInfo();
    void createEditNote();
    void createEditHubInfo();
    void createShowRefFreqResponse();
    void createEditRefFreqResponse();

    using Func = void (MainWindow::*)();

    static constexpr Func createInfoFun[]{&MainWindow::createShowInfo,
                                          &MainWindow::createEditInfo,
                                          &MainWindow::createEditNote,
                                          &MainWindow::createEditHubInfo,
                                          &MainWindow::createShowRefFreqResponse,
                                          &MainWindow::createEditRefFreqResponse};

    void createInfo(int i) { (this->*createInfoFun[i])();}

};
#endif // MAINWINDOW_H
