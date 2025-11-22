#ifndef EXECUTER_H
#define EXECUTER_H

#include <QWidget>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProcess>
#include <QString>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>
#include <QFileInfo>
#include <QTextEdit>

class Executer : public QObject
{
    Q_OBJECT

public:
    static void executePy(const QString &currentFilePath, QWidget *parent);
};

#endif // EXECUTER_H