#include "executer.h"

void Executer::executePy(const QString &currentFilePath, QWidget *parent) {
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(parent, "Error", "Please save the file first");
        return;
    }
    
    QWidget *runnerWindow = new QWidget();
    runnerWindow->setWindowTitle("Runner - " + QFileInfo(currentFilePath).fileName());
    runnerWindow->setMinimumSize(600, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(runnerWindow);
    
    QTextEdit *outputWidget = new QTextEdit();
    outputWidget->setReadOnly(true);
    mainLayout->addWidget(outputWidget);
    
    QWidget *inputWidget = new QWidget();
    QHBoxLayout *inputLayout = new QHBoxLayout(inputWidget);
    inputLayout->setContentsMargins(5, 5, 5, 5);
    
    QLabel *inputLabel = new QLabel("Input:");
    QLineEdit *inputLineEdit = new QLineEdit();
    QPushButton *sendButton = new QPushButton("Send");
    
    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(inputLineEdit);
    inputLayout->addWidget(sendButton);
    
    mainLayout->addWidget(inputWidget);
    
    inputWidget->setVisible(false);
    
    QProcess *process = new QProcess(runnerWindow);
    
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process, outputWidget, inputWidget]() {
        QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
        
        if (output.contains(":") || output.toLower().contains("input") || output.trimmed().endsWith(":")) {
            inputWidget->setVisible(true);
        }
        
        outputWidget->append(output);
    });
    
    QObject::connect(process, &QProcess::readyReadStandardError, [process, outputWidget]() {
        QString error = QString::fromLocal8Bit(process->readAllStandardError());
        outputWidget->append("<font color='red'>" + error + "</font>");
    });
    
    QObject::connect(sendButton, &QPushButton::clicked, [process, inputLineEdit, outputWidget, inputWidget]() {
        QString inputText = inputLineEdit->text() + "\n";
        process->write(inputText.toLocal8Bit());
        inputLineEdit->clear();
        inputWidget->setVisible(false);
    });
    
    QObject::connect(inputLineEdit, &QLineEdit::returnPressed, sendButton, &QPushButton::click);
    
    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [outputWidget, process, inputWidget, runnerWindow](int exitCode, QProcess::ExitStatus exitStatus) {
                outputWidget->append("\n----------------------------------------");
                outputWidget->append(QString("Process finished with exit code: %1").arg(exitCode));
                inputWidget->setVisible(false);
                process->deleteLater();
            });
    
    runnerWindow->show();
    outputWidget->append("File: " + currentFilePath);
    outputWidget->append("----------------------------------------\n");
    
    process->setProcessChannelMode(QProcess::MergedChannels);
    
    QStringList pythonCommands = {"python", "python3", "py"};
    bool processStarted = false;
    
    for (const QString &command : pythonCommands) {
        process->start(command, QStringList() << "-i" << currentFilePath);
        
        if (process->waitForStarted(2000)) {
            processStarted = true;
            break;
        }
    }
    
    if (!processStarted) {
        outputWidget->append("\n<font color='red'>ERROR: Could not start Python interpreter!</font>");
        outputWidget->append("Please make sure Python is installed and available in your PATH.");
        outputWidget->append("You can download Python from: https://www.python.org/downloads/");
        
        QMessageBox::critical(parent, "Python Not Found", 
            "Could not find Python interpreter on your system.\n\n"
            "Please install Python and make sure it's available in your PATH environment variable.\n"
            "Download from: https://www.python.org/downloads/");
    }
}