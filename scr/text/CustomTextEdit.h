#include <QTextEdit>
#include <QKeyEvent>
#include <QTextCursor>

class CustomTextEdit : public QTextEdit {
public:
    explicit CustomTextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Tab) {
            insertPlainText("    ");
            event->accept();
        } else if (event->key() == Qt::Key_Backspace) {
            handleBackspace();
            event->accept();
        } else if (event->key() == Qt::Key_ParenLeft) {
            autoParens();
            event->accept();
        } else if (event->key() == Qt::Key_Apostrophe) {
            autoSingleStrings();
            event->accept();
        } else if (event->key() == Qt::Key_QuoteDbl) {
            autoDoubleStrings();
            event->accept();
        } else if (event->key() == Qt::Key_BraceLeft) {
            autoBraces();
            event->accept();
        } else if (event->key() == Qt::Key_BracketLeft) {
            autoSquareBrackets();
            event->accept();
        } else {
            QTextEdit::keyPressEvent(event);
        }
    }

private:
    void autoParens() {
        QTextCursor cursor = textCursor();

        cursor.insertText("()");

        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }

    void autoDoubleStrings() {
        QTextCursor cursor = textCursor();

        cursor.insertText("\"\"");

        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }
    
    void autoSingleStrings() {
        QTextCursor cursor = textCursor();

        cursor.insertText("\'\'");

        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }

    void autoBraces() {
        QTextCursor cursor = textCursor();

        cursor.insertText("{}");

        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }

    void autoSquareBrackets() {
        QTextCursor cursor = textCursor();

        cursor.insertText("[]");

        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }

    

    void handleBackspace() {
        QTextCursor cursor = textCursor();
        
        if (cursor.hasSelection()) {
            cursor.removeSelectedText();
            return;
        }
        
        int position = cursor.position();
        
        if (position > 0) {
            cursor.movePosition(QTextCursor::Left);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
            QString selectedText = cursor.selectedText();
            
            if (selectedText == "()") {
                cursor.removeSelectedText();
                return;
            } else if (selectedText == "\"\"") {
                cursor.removeSelectedText();
                return;
            } else if (selectedText == "\'\'") {
                cursor.removeSelectedText();
                return;
            } else if (selectedText == "{}") {
                cursor.removeSelectedText();
                return;
            } else if (selectedText == "[]") {
                cursor.removeSelectedText();
                return;
            }
        }
        
        cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, position - cursor.position());
        QString textBeforeCursor = cursor.selectedText();
        
        int spacesCount = 0;
        for (int i = textBeforeCursor.length() - 1; i >= 0; --i) {
            if (textBeforeCursor.at(i) == ' ') {
                spacesCount++;
            } else {
                break;
            }
        }
        
        if (spacesCount >= 4 && spacesCount % 4 == 0) {
            cursor = textCursor();
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 4);
            if (cursor.selectedText() == "    ") {
                cursor.removeSelectedText();
                return;
            }
        }
        
        cursor = textCursor();
        cursor.deletePreviousChar();
    }
};