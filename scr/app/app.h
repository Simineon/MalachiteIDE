#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include "../text/CustomTextEdit.h"

class App : public QWidget {
    Q_OBJECT

public:
    App(QWidget *parent = nullptr);

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void exitApp();
    void executePy();

private:
    CustomTextEdit *editor;
    QString currentFilePath;
};

#endif 