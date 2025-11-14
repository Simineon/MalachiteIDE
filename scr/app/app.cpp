#include "app.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTemporaryFile>
#include <QDebug>
#include "../parser/parser.h" 

App::App(QWidget *parent) : QWidget(parent) {
    QMenuBar *menuBar = new QMenuBar(this);
    editor = new CustomTextEdit(this);

    // Creeating menus
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QMenu *runMenu = menuBar->addMenu(tr("&Run"));
    
    // File Menu
    QAction *newAction = fileMenu->addAction(tr("&New"));
    QAction *openAction = fileMenu->addAction(tr("&Open"));
    QAction *saveAction = fileMenu->addAction(tr("&Save"));
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));

    // Run Menu 
    QAction *runCurrentFile = runMenu->addAction(tr("&Run current file and save"));
    runMenu->addSeparator();
    exitAction = runMenu->addAction(tr("E&xit"));
    
    // Connect Actions
    connect(newAction, &QAction::triggered, this, &App::newFile);
    connect(openAction, &QAction::triggered, this, &App::openFile);
    connect(saveAction, &QAction::triggered, this, &App::saveFile);
    connect(exitAction, &QAction::triggered, this, &App::exitApp);
    connect(runCurrentFile, &QAction::triggered, this, &App::executePy);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(menuBar);
    layout->addWidget(editor);

    editor->setPlainText(
        "def hello_world():\n"
        "    # Это комментарий\n"
        "    print(\"Hello, World!\")\n"
        "    x = 42\n"
        "    if x > 10:\n"
        "        return True\n"
        "    else:\n"
        "        return False\n"
        "\n"
        "class MyClass:\n"
        "    def __init__(self):\n"
        "        self.value = 100\n"
        "\n"
        "# Добавим вызов функции для тестирования\n"
        "hello_world()\n"
    );

    // highlighting
    Parser *highlighter = new Parser(editor->document());

    // Window settings
    setWindowTitle("Malachite IDE");
    setMinimumSize(500, 500);
    setMaximumSize(1600, 1000);
}

void App::newFile() {
    editor->clear();
    currentFilePath.clear();
    setWindowTitle("Malachite IDE - New file");
}

void App::openFile() {
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Open file", "", "Python files (*.py);;Text files (*.txt);;All files (*)");

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString fileContent = in.readAll();
            editor->setPlainText(fileContent);
            file.close();
            currentFilePath = filePath;
            setWindowTitle("Malachite IDE - " + filePath);
        } else {
            QMessageBox::warning(this, "Error", "Error in file opening!");
        }
    }
}

void App::saveFile() {
    QString fileName;
    
    if (currentFilePath.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(
            editor,
            "Save file",
            "",
            "Python files (*.py);;Text files (*.txt);;All files (*)"
        );
        if (!fileName.isEmpty()) {
            // Добавляем расширение .py если его нет
            if (!fileName.endsWith(".py") && !fileName.endsWith(".txt")) {
                fileName += ".py";
            }
            currentFilePath = fileName;
        }
    } else {
        fileName = currentFilePath;
    }

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << editor->toPlainText();
            file.close();
            setWindowTitle("Malachite IDE - " + fileName);
        } else {
            QMessageBox::warning(this, "Error", "Error in file saving!");
        }
    }
}

void App::executePy() {
    // in beggining we saving file
    saveFile();
    
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please save the file first");
        return;
    }
    
    // results
    QTextEdit *outputWidget = new QTextEdit();
    outputWidget->setWindowTitle("Runner - " + QFileInfo(currentFilePath).fileName());
    outputWidget->setMinimumSize(600, 400);
    outputWidget->setReadOnly(true);
    
    outputWidget->clear();
    
    QProcess *process = new QProcess(this);
    
    // Подключаем обработчики вывода в реальном времени
    connect(process, &QProcess::readyReadStandardOutput, [process, outputWidget]() {
        QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
        outputWidget->append(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, [process, outputWidget]() {
        QString error = QString::fromLocal8Bit(process->readAllStandardError());
        outputWidget->append("<font color='red'>" + error + "</font>");
    });
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [outputWidget, process, this](int exitCode, QProcess::ExitStatus exitStatus) {
                outputWidget->append("\n----------------------------------------");
                outputWidget->append(QString("Process finished with exit code: %1").arg(exitCode));
                process->deleteLater();
            });
    
    // Show runner window
    outputWidget->show();
    outputWidget->append("File: " + currentFilePath);
    outputWidget->append("----------------------------------------\n");
    
    // Different commands for launch Python
    QStringList pythonCommands = {"python", "python3", "py"};
    bool processStarted = false;
    
    for (const QString &command : pythonCommands) {
        process->start(command, QStringList() << currentFilePath);
        
        if (process->waitForStarted(2000)) {
            outputWidget->append(QString("Successfully started with: %1").arg(command));
            processStarted = true;
            break;
        } else {
            outputWidget->append(QString("Failed to start with: %1").arg(command));
        }
    }
    
    if (!processStarted) {
        outputWidget->append("\n<font color='red'>ERROR: Could not start Python interpreter!</font>");
        outputWidget->append("Please make sure Python is installed and available in your PATH.");
        outputWidget->append("You can download Python from: https://www.python.org/downloads/");
        
        QMessageBox::critical(this, "Python Not Found", 
            "Could not find Python interpreter on your system.\n\n"
            "Please install Python and make sure it's available in your PATH environment variable.\n"
            "Download from: https://www.python.org/downloads/");
    }
}

void App::exitApp() {
    QApplication::quit();
}