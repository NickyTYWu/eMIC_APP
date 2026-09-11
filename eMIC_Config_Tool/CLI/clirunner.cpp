#include "clirunner.h"
#include "mainwindow.h" // Includes MainWindow context for hardware linkage
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTableWidget>
#include <QComboBox>
#include <QTimer>
#include <QTextStream>
#include <QEventLoop>
#include <windows.h> // Required for AttachConsole API
#include "global.h"
#include "Command.h"

CliRunner::CliRunner(const QStringList &args, QObject *parent)
    : QObject(parent), m_args(args), m_w(nullptr)
{
}

CliRunner::~CliRunner()
{
    // Securely release the hidden MainWindow instance when CliRunner finishes
    if (m_w) {
        delete m_w;
    }
}

void CliRunner::doFinished(bool success)
{
#ifdef Q_OS_WIN
    INPUT inputs = {};
    ZeroMemory(&inputs, sizeof(inputs));
    inputs.type = INPUT_KEYBOARD;
    inputs.ki.wVk = VK_RETURN;
    SendInput(1, &inputs, sizeof(INPUT));
    inputs.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &inputs, sizeof(INPUT));
#endif

    emit finished(success);
}

void CliRunner::run()
{
    // Attach standard outputs back to the active parent CMD window
    AttachConsole(ATTACH_PARENT_PROCESS);

    if (m_args.contains("--help") || m_args.contains("-h") || m_args.contains("-?")) {
        outMSG << "\n\n"
               << "Usage: eMIC_Config_Tool.exe [options]\n\n"
               << "===============================================================================\n"
               << " eMIC Register Configuration Pipeline Utility - Command Guidelines\n"
               << "===============================================================================\n\n"
               << "[Category 1] Firmware & Diagnostics\n"
               << "-------------------------------------------------------------------------------\n"
               << " * Read firmware version:\n"
               << "   eMIC_Config_Tool.exe --cli --port COM19 --channel 0 --action read-version\n\n"
               << " * Read physical device identification (devID):\n"
               << "   eMIC_Config_Tool.exe --cli --port COM19 --channel 0 --action read-deviceid\n\n"
               << "[Category 2] Climate & Telemetry Sensor Read\n"
               << "-------------------------------------------------------------------------------\n"
               << " * Read SHT4x humidity & temperature data:\n"
               << "   eMIC_Config_Tool.exe --cli --port COM19 --channel 0 --action read-sensor --type sht4x\n\n"
               << " * Read DSP368 pressure & temperature data:\n"
               << "   eMIC_Config_Tool.exe --cli --port COM19 --channel 0 --action read-sensor --type dsp368\n\n"
               << "[Category 3] Data Source: EEPROM Cache (Non-Volatile Memory Sync)\n"
               << "-------------------------------------------------------------------------------\n"
               << " * Read partition block layout data from inner EEPROM:\n"
               << "   eMIC_Config_Tool.exe --cli --port COM19 --channel 0 --action read-page --page 2\n\n"
               << " * Batch upload and commit local CSV data map table directly to EEPROM:\n"
               << "   eMIC_Config_Tool.exe --cli --port COM19 --channel 0 --action write-page --page 2 --csv .\\default_config_csv\\page2.csv\n\n"
               << "[Category 4] Data Source: PCMD3140 Register Direct Access (Live Hardware)\n"
               << "-------------------------------------------------------------------------------\n"
               << " * Live query immediate runtime register cell matrix from PCMD3140 chip:\n"
               << "   eMIC_Config_Tool.exe --cli --port COM19 --channel 0 --action pcmd-read-page --page 2\n\n"
               << " * Force execute active register block hot-override override stream onto PCMD3140 chip:\n"
               << "   eMIC_Config_Tool.exe --cli --port COM19 --channel 0 --action pcmd-write-page --page 2 --csv .\\default_config_csv\\page2.csv\n\n"
               << "===============================================================================\n"
               << "\nOptions:\n"
               << "  --cli                 Execute in command line interface mode.\n"
               << "  --action <action>     Action:\n"
               << "                        'write-page','read-page','pcmd-write-page','pcmd-read-page',\n"
               << "                        'read-version','read-deviceid','read-sensor'.\n"
               << "  --port <port>         Specify serial port channel(e.g., COM19).\n"
               << "  --page <page>         Target register page (0, 2, 3, 4).\n"
               << "  --csv <csv>           Import configuration CSV path.\n"
               << "  --channel <channel>   Select UART channel (0-3).\n"
               << "  --type <type>         Specify sensor hardware identity ('sht4x' / 'dsp368').\n"
               << "  -?, -h, --help        Displays help on commandline options.\n" << Qt::endl;

        emit finished(true);
        return;
    }
    // =========================================================================
    // 👑 Gate 1: One-Shot Structural Validation
    // =========================================================================
    // Check all required arguments that must expect a trailing value block
    QStringList requiredValueOptions = {"--port", "--action", "--page", "--csv", "--channel", "--type"};

    for (int i = 0; i < m_args.size(); ++i) {
        QString currentArg = m_args.at(i);

        if (requiredValueOptions.contains(currentArg)) {
            // Guard A: Next index bounds overflow check
            if (i + 1 >= m_args.size()) {
                outMSG << "\nError: Critical syntax error! The parameter '" << currentArg
                       << "' expects a value but none was provided." << Qt::endl;
                doFinished(false);
                return;
            }

            // Guard B: Misalignment check to prevent parameter swallow
            QString nextArg = m_args.at(i + 1);
            if (nextArg.startsWith("-")) {
                outMSG << "\nError: Critical syntax error! Detected parameter misalignment. "
                       << "The argument '" << currentArg << "' cannot be followed by another flag '" << nextArg
                       << "'. Please provide a valid value for '" << currentArg << "'." << Qt::endl;
                doFinished(false);
                return; // Early abort right here before QCommandLineParser processes it
            }
        }
    } // 📌 S

    // Arguments validation passed. Now initialize the parser
    QCommandLineParser parser;
    //parser.setApplicationDescription("eMIC Register Configuration Tool (CLI Mode)");

    QCommandLineOption cliOption("cli", "Execute in command line interface mode.");
    QCommandLineOption actionOption("action", "Action: 'write-page','read-page','pcmd-write-page,'pcmd-read-page','read-version','read-deviceid','read-sensor'.", "action");
    QCommandLineOption portOption("port", "Specify serial port channel.", "port");
    QCommandLineOption pageOption("page", "Target register page (0, 2, 3, 4).", "page");
    QCommandLineOption csvOption("csv", "Import configuration CSV path.", "csv");
    QCommandLineOption channelOption("channel", "Select UART channel (0-3).", "channel", "0");
    QCommandLineOption sensorOption("type", "Specify sensor hardware identity.", "type");

    parser.addOption(cliOption);
    parser.addOption(actionOption);
    parser.addOption(portOption);
    parser.addOption(pageOption);
    parser.addOption(csvOption);
    parser.addOption(channelOption);
    parser.addOption(sensorOption);
    //parser.addHelpOption();

    parser.process(m_args);

    // =========================================================================
    // 👑 Gate 2: Business Logic Validation Block
    // =========================================================================
    if (!parser.isSet(portOption)) {
        outMSG << "\n[Feedback]Error: Required parameter --port <COM> is missing." << Qt::endl;
        doFinished(false); return;
    }

    if (!parser.isSet(actionOption)) {
        outMSG << "\n[Feedback]Error: Missing required parameter --action.\n"
               << "Available options: write-page, read-version, read-page, read-sensor" << Qt::endl;
        doFinished(false); return;
    }

    QString portName = parser.value(portOption);
    QString action = parser.value(actionOption).toLower();
    QString channelRawValue = parser.value(channelOption);

    // Regular Expression validation for channel index bounds
    static QRegularExpression digitRx("^[0-3]$");
    if (!digitRx.match(channelRawValue).hasMatch()) {
        outMSG << "\n[Feedback]Error: Invalid UART channel input. Must be 0, 1, 2, or 3." << Qt::endl;
        doFinished(false); return;
    }
    int channelNum = channelRawValue.toInt();

    // Deep sub-validation routing checks
    if (action == "write-page" || action == "read-page"|| action == "pcmd-read-page"|| action == "pcmd-write-page") {
        if (!parser.isSet(pageOption)) {
            outMSG << "\n[Feedback]Error: Action '" << action << "' requires parameter --page <0|2|3|4>." << Qt::endl;
            doFinished(false); return;
        }

        int pageNum = parser.value(pageOption).toInt();
        if (pageNum != 0 && pageNum != 2 && pageNum != 3 && pageNum != 4) {
            outMSG << "\n[Feedback]Error: Action '" << action << "' requires parameter --page <0|2|3|4>." << Qt::endl;
            doFinished(false); return;
        }

        if (action == "write-page" && !parser.isSet(csvOption)) {
            outMSG << "\n[Feedback]Error: Action 'write-page' requires parameter --csv <path>." << Qt::endl;
            doFinished(false); return;
        }
    }
    else if (action == "read-sensor") {
        if (!parser.isSet(sensorOption)) {
            outMSG << "\n[Feedback]Error: Action 'read-sensor' requires parameter --type <sht4x|dsp368>." << Qt::endl;
            doFinished(false); return;
        }
        QString sType = parser.value(sensorOption).toLower();
        if (sType != "sht4x" && sType != "dsp368") {
            outMSG << "\n[Feedback]Error: Sensor profile identity unsupported." << Qt::endl;
            doFinished(false); return;
        }
    }
    // =========================================================================
    // Validation passed! Construct the hidden MainWindow instance on Heap memory
    // =========================================================================
    m_w = new MainWindow();

    // Verify whether the designated COM port exists inside the UI list context
    QComboBox* comBox = m_w->findChild<QComboBox*>("comPort_comboBox");
    int comboIndex = comBox ? comBox->findText(portName) : -1;
    if (comboIndex == -1) {
        outMSG << "\n[Feedback]Error: Specified port '" << portName << "' was not detected by Windows." << Qt::endl;
        doFinished(false);
        return;
    }
    comBox->setCurrentIndex(comboIndex);

    // Setup an isolated asynchronous QEventLoop synchronization monitor grid
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    bool isTaskSuccess = false;

    connect(m_w, &MainWindow::cliCommandCompleted, &loop, [&](bool success) {
        isTaskSuccess = success;
        loop.quit();
    });

    connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        outMSG << "\n[Feedback]Error: Timeout waiting for hardware verification response (5000ms elapsed)." << Qt::endl;
        isTaskSuccess = false;
        loop.quit();
    });

    timeoutTimer.start(5000);

    m_w->connectBtn_clicked();
    //QCoreApplication::processEvents();

    // Switch multiplexer path and invoke hardware initialization routines
    m_w->updateUartChannel(channelNum);

    // Intercept hardware logic if port failed to open properly
    if (m_w && m_w->mSerial && m_w->mSerial->isOpen()) {

        if (action == "write-page") {
            int pageNum = parser.value(pageOption).toInt();
            QString csvFile = parser.value(csvOption);
            QTableWidget* targetTable = m_w->findChild<QTableWidget*>(QString("tableWidget_page%1").arg(pageNum));

            if (!targetTable || !m_w->checkImportCSVFormat(targetTable, csvFile)) {
                outMSG << "\n[Feedback]Error: Target configuration setup context or CSV layout verification failed." << Qt::endl;
                emit finished(false);
                return;
            }
            m_w->importFromCSV(targetTable, csvFile);

            if (pageNum == 0)      m_w->savePage0();
            else if (pageNum == 2) m_w->savePage2();
            else if (pageNum == 3) m_w->savePage3();
            else if (pageNum == 4) m_w->savePage4();
        }
        else if (action == "read-page") {
            int pageNum = parser.value(pageOption).toInt();
            if (pageNum == 0)      pageNum = BLOCK_ID_PAGE0;
            else if (pageNum == 2) pageNum = BLOCK_ID_PAGE2;
            else if (pageNum == 3) pageNum = BLOCK_ID_PAGE3;
            else if (pageNum == 4) pageNum = BLOCK_ID_PAGE4;

            m_w->refreshPage(pageNum, 0);
        }
        else if (action == "pcmd-write-page") {
            int pageNum = parser.value(pageOption).toInt();
            QString csvFile = parser.value(csvOption);
            QTableWidget* targetTable = m_w->findChild<QTableWidget*>(QString("tableWidget_page%1").arg(pageNum));

            if (!targetTable || !m_w->checkImportCSVFormat(targetTable, csvFile)) {
                outMSG << "\n[Feedback]Error: Target configuration setup context or CSV layout verification failed." << Qt::endl;
                emit finished(false);
                return;
            }

            m_w->importFromCSV(targetTable, csvFile);

            if (pageNum == 0)      m_w->page0WriteAllBtn_clicked();
            else if (pageNum == 2) m_w->page2WriteAllBtn_clicked();
            else if (pageNum == 3) m_w->page3WriteAllBtn_clicked();
            else if (pageNum == 4) m_w->page4WriteAllBtn_clicked();
        }
        else if (action == "pcmd-read-page") {
            int pageNum = parser.value(pageOption).toInt();
            //outMSG << "Pulling memory map dump from Register Page " << parser.value(pageOption) << "..." << Qt::endl;
            if (pageNum == 0)      pageNum = BLOCK_ID_PAGE0;
            else if (pageNum == 2) pageNum = BLOCK_ID_PAGE2;
            else if (pageNum == 3) pageNum = BLOCK_ID_PAGE3;
            else if (pageNum == 4) pageNum = BLOCK_ID_PAGE4;
            m_w->refreshPage(pageNum, 1);
        }
        else if (action == "read-version") {
            m_w->GenCommand(GET_FW_VERSION_CMD, NULL, 0);
        }
        else if (action == "read-deviceid") {
            m_w->refreshInfo();
        }
        else if (action == "read-sensor") {
            QString sType = parser.value(sensorOption).toLower();
            if (sType == "sht4x") {
                m_w->readOutSHT4x();
            } else if (sType == "dsp368") {
                m_w->GenCommand(READ_DPS368_COEFF_CMD, NULL, 0);
            }
        }
    } else {
        outMSG << "\n[Feedback]Aborting operational routine stack due to channel connection failure." << Qt::endl;
        isTaskSuccess = false;
        loop.quit();
    }

    loop.exec();

    if (isTaskSuccess) {
        outMSG << "\n[Feedback]Success: Transaction finalized successfully. Terminating execution context." << Qt::endl;
    } else {
        outMSG << "\n[Feedback]Failure: Operational routine dropped by error." << Qt::endl;
    }

    doFinished(isTaskSuccess);
}
