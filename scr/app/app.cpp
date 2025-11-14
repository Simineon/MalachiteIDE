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
#include "../parser/parser.h" 

App::App(QWidget *parent) : QWidget(parent) {
    QMenuBar *menuBar = new QMenuBar(this);
    editor = new CustomTextEdit(this);

    // Создаем меню
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    
    // Добавляем действия в меню File
    QAction *newAction = fileMenu->addAction(tr("&New"));
    QAction *openAction = fileMenu->addAction(tr("&Open"));
    QAction *saveAction = fileMenu->addAction(tr("&Save"));
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    
    // Подключаем действия
    connect(newAction, &QAction::triggered, this, &App::newFile);
    connect(openAction, &QAction::triggered, this, &App::openFile);
    connect(saveAction, &QAction::triggered, this, &App::saveFile);
    connect(exitAction, &QAction::triggered, this, &App::exitApp);
    
    // Добавляем меню-бар и редактор в макет
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
    );

    // СОЗДАЕМ И ПРИМЕНЯЕМ ПАРСЕР ДЛЯ ПОДСВЕТКИ СИНТАКСИСА
    Parser *highlighter = new Parser(editor->document());

    // Настраиваем окно
    setWindowTitle("Malachite IDE");
    setMinimumSize(500, 500);
    setMaximumSize(1600, 1000);
}

void App::newFile() {
    editor->clear();
    setWindowTitle("Malachite IDE - New files");
}

void App::openFile() {
    // We opens dialog window for choose file(s)
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Open file", "", "Text files (*.txt);;Python files (*.py);;All files (*)");

    // Checking, choosed file?
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            // Read file content
            QString fileContent = in.readAll();
            editor->setPlainText(fileContent);
            file.close();
        } else {
            editor->setPlainText("Ошибка открытия файла.");
        }
    }
}

void App::saveFile() {
    QString fileName = QFileDialog::getSaveFileName(
        editor,
        "Save file",
        "",
        "Text files (*.txt);;Python files (*.py);;All files (*)"
    );

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << editor->toPlainText();
            file.close();
        }
    }
}

void App::exitApp() {
    QApplication::quit();
}