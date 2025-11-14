#include <QApplication>
#include "scr/app/app.h"
#include "scr/parser/parser.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    App window;
    window.show();
    
    return app.exec();
}