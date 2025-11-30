#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QTextCursor>
#include <QWidget>
#include <QTextBlock>
#include <QCompleter>
#include <QStringList>
#include <QScrollBar>
#include <QAbstractItemView>

class CustomTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CustomTextEdit(QWidget *parent = nullptr);
    ~CustomTextEdit();
    
    void setCompleter(QCompleter *completer);
    QCompleter *completer() const { return c; }
    
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private slots:
    void insertCompletion(const QString &completion);

private:
    void autoParens();
    void autoDoubleStrings();
    void autoSingleStrings();
    void autoBraces();
    void autoSquareBrackets();
    void handleBackspace();
    void handleEnter();
    void createTips();
    QString textUnderCursor() const;
    
private:
    QCompleter *c = nullptr;
};

inline CustomTextEdit::CustomTextEdit(QWidget *parent) : 
    QPlainTextEdit(parent)
{
    // Font setup
    QFont font("Consolas", 12);
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    
    // Optimization
    setCenterOnScroll(false);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    
    // Create completer
    createTips();
}

inline CustomTextEdit::~CustomTextEdit() {
    if (c) {
        delete c;
    }
}

inline void CustomTextEdit::setCompleter(QCompleter *completer) {
    if (c) {
        disconnect(c, nullptr, this, nullptr);
        delete c;
    }
    
    c = completer;
    if (!c) {
        return;
    }
    
    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    
    connect(c, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CustomTextEdit::insertCompletion);
}

inline void CustomTextEdit::focusInEvent(QFocusEvent *event) {
    if (c) {
        c->setWidget(this);
    }
    QPlainTextEdit::focusInEvent(event);
}

inline void CustomTextEdit::insertCompletion(const QString &completion) {
    if (!c || c->widget() != this) {
        return;
    }
    
    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

inline QString CustomTextEdit::textUnderCursor() const {
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

inline void CustomTextEdit::keyPressEvent(QKeyEvent *event) {
    // Handle completer
    if (c && c->popup() && c->popup()->isVisible()) {
        switch (event->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                event->ignore();
                return;
            default:
                break;
        }
    }
    
    bool isShortcut = ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_Space);
    
    // Special keys
    switch (event->key()) {
        case Qt::Key_Tab:
            if (c && c->popup() && c->popup()->isVisible()) {
                c->popup()->hide();
                event->accept();
                return;
            }
            insertPlainText("    ");
            event->accept();
            return;
        case Qt::Key_Backspace:
            handleBackspace();
            event->accept();
            return;
        case Qt::Key_ParenLeft:
            autoParens();
            event->accept();
            return;
        case Qt::Key_Apostrophe:
            if (event->modifiers() == Qt::NoModifier) {
                autoSingleStrings();
                event->accept();
                return;
            }
            break;
        case Qt::Key_QuoteDbl:
            if (event->modifiers() == Qt::NoModifier) {
                autoDoubleStrings();
                event->accept();
                return;
            }
            break;
        case Qt::Key_BraceLeft:
            autoBraces();
            event->accept();
            return;
        case Qt::Key_BracketLeft:
            autoSquareBrackets();
            event->accept();
            return;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            handleEnter();
            event->accept();
            return;
        default:
            break;
    }
    
    // Process the key normally first
    QPlainTextEdit::keyPressEvent(event);
    
    // Handle completer after normal processing
    if (!c) {
        return;
    }
    
    const bool ctrlOrShift = event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (ctrlOrShift && event->text().isEmpty()) {
        return;
    }
    
    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();
    
    // Don't show completer in these cases
    if (!isShortcut && (hasModifier || event->text().isEmpty() || 
                       completionPrefix.length() < 1 ||  // Изменено с 2 на 1
                       eow.contains(event->text().right(1)))) {
        if (c->popup()) {
            c->popup()->hide();
        }
        return;
    }
    
    // Update completer
    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        if (c->popup()) {
            c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
        }
    }
    
    // Only show popup if there are completions and we have enough characters
    if (completionPrefix.length() >= 1 && c->completionCount() > 0) {  // Изменено с 2 на 1
        QRect cr = cursorRect();
        cr.setWidth(c->popup()->sizeHintForColumn(0) 
                    + c->popup()->verticalScrollBar()->sizeHint().width());
        c->complete(cr);
    } else {
        if (c->popup()) {
            c->popup()->hide();
        }
    }
}

inline void CustomTextEdit::handleEnter() {
    QTextCursor cursor = textCursor();
    
    // Getting current block (line)
    QTextBlock currentBlock = cursor.block();
    QString currentLineText = currentBlock.text();
    
    int indentCount = 0;
    while (indentCount < currentLineText.length() && currentLineText.at(indentCount) == ' ') {
        indentCount++;
    }
    
    // Check on def or class
    bool shouldAddExtraIndent = false;
    QString trimmedLine = currentLineText.trimmed();
    if (trimmedLine.startsWith("class ") || trimmedLine.startsWith("def ")) {
        shouldAddExtraIndent = true;
    }
    
    // Insert new line
    QPlainTextEdit::keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier));
    
    int newIndentCount = indentCount;
    if (shouldAddExtraIndent) {
        newIndentCount += 4;
    }
    
    if (newIndentCount > 0) {
        cursor = textCursor();
        QString indent(newIndentCount, ' ');
        cursor.insertText(indent);
    }
}

inline void CustomTextEdit::autoParens() {
    QTextCursor cursor = textCursor();
    cursor.insertText("()");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::autoDoubleStrings() {
    QTextCursor cursor = textCursor();
    cursor.insertText("\"\"");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::autoSingleStrings() {
    QTextCursor cursor = textCursor();
    cursor.insertText("''");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::autoBraces() {
    QTextCursor cursor = textCursor();
    cursor.insertText("{}");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::autoSquareBrackets() {
    QTextCursor cursor = textCursor();
    cursor.insertText("[]");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::handleBackspace() {
    QTextCursor cursor = textCursor();
    
    if (cursor.hasSelection()) {
        cursor.removeSelectedText();
        return;
    }
    
    int position = cursor.position();
    
    // Chacking
    if (position > 0) {
        cursor.movePosition(QTextCursor::Left);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
        QString selectedText = cursor.selectedText();
        
        if (selectedText == "()" || selectedText == "\"\"" || 
            selectedText == "''" || selectedText == "{}" || selectedText == "[]") {
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

inline void CustomTextEdit::createTips() {
    QStringList pythonKeyWord;
    pythonKeyWord << "def" << "class" << "input" << "print" << "if" << "elif" << "else" << "int" << "float" << "str" 
                  << "bool" << "break" << "continue" << "for" << "while" << "return" << "import" << "from" << "as" 
                  << "try" << "except" << "finally" << "with" << "lambda" << "None" << "True" << "False" << "pass"
                  << "in" << "del" << "range" << "max";

    QCompleter *completer = new QCompleter(pythonKeyWord, this);
    setCompleter(completer);
}

#endif // CUSTOMTEXTEDIT_H