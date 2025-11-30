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
#include <QPaintEvent>
#include <QTextEdit>
#include <QTextDocument>
#include <QPainter>

// Forward declaration
class CustomTextEdit;

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CustomTextEdit *editor);
    
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CustomTextEdit *textEdit;
};

class CustomTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CustomTextEdit(QWidget *parent = nullptr);
    ~CustomTextEdit();
    
    void setCompleter(QCompleter *completer);
    QCompleter *completer() const { return c; }
    
    // Methods for line numbering
    int lineNumberAreaWidth();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void resizeEvent(QResizeEvent *event) override;
    
    // Style methods for line numbers
    void setLineNumberAreaBackground(const QColor &color);
    void setLineNumberColor(const QColor &color);
    void setCurrentLineHighlight(const QColor &color);
    void setLineNumberFont(const QFont &font);
    void setLineNumberAlignment(Qt::Alignment alignment);
    void setLineNumberMargin(int margin);
    
    QColor lineNumberAreaBackground() const { return lineNumberBgColor; }
    QColor lineNumberColor() const { return lineNumberTextColor; }
    QColor currentLineHighlight() const { return currentLineColor; }
    QFont lineNumberFont() const { return lineNumberAreaFont; }
    Qt::Alignment lineNumberAlignment() const { return lineNumberAlign; }
    int lineNumberMargin() const { return lineNumberMarginPx; }

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
    LineNumberArea *lineNumberArea;
    
    // Style properties for line numbers
    QColor lineNumberBgColor = QColor(240, 240, 240);
    QColor lineNumberTextColor = Qt::black;
    QColor currentLineColor = QColor(200, 200, 255);
    QFont lineNumberAreaFont;
    Qt::Alignment lineNumberAlign = Qt::AlignRight;
    int lineNumberMarginPx = 5;
};

inline LineNumberArea::LineNumberArea(CustomTextEdit *editor) : QWidget(editor), textEdit(editor) {}

inline QSize LineNumberArea::sizeHint() const {
    return QSize(textEdit->lineNumberAreaWidth(), 0);
}

inline void LineNumberArea::paintEvent(QPaintEvent *event) {
    textEdit->lineNumberAreaPaintEvent(event);
}

inline CustomTextEdit::CustomTextEdit(QWidget *parent) : 
    QPlainTextEdit(parent)
{
    // Font setup
    QFont font("Consolas", 12);
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    
    // Set line number font (can be different from main font)
    lineNumberAreaFont = font;
    lineNumberAreaFont.setPointSize(font.pointSize() - 1); // Slightly smaller
    
    // Optimization
    setCenterOnScroll(false);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    
    // Create completer
    createTips();
    
    lineNumberArea = new LineNumberArea(this);
    
    connect(this, &QPlainTextEdit::blockCountChanged, this, &CustomTextEdit::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CustomTextEdit::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
        viewport()->update();
    });
    
    updateLineNumberAreaWidth(0);
}

inline CustomTextEdit::~CustomTextEdit() {
    if (c) {
        delete c;
    }
    if (lineNumberArea) {
        delete lineNumberArea;
    }
}

// Style methods implementation
inline void CustomTextEdit::setLineNumberAreaBackground(const QColor &color) {
    lineNumberBgColor = color;
    if (lineNumberArea) {
        lineNumberArea->update();
    }
}

inline void CustomTextEdit::setLineNumberColor(const QColor &color) {
    lineNumberTextColor = color;
    if (lineNumberArea) {
        lineNumberArea->update();
    }
}

inline void CustomTextEdit::setCurrentLineHighlight(const QColor &color) {
    currentLineColor = color;
    if (lineNumberArea) {
        lineNumberArea->update();
    }
}

inline void CustomTextEdit::setLineNumberFont(const QFont &font) {
    lineNumberAreaFont = font;
    if (lineNumberArea) {
        lineNumberArea->update();
        updateLineNumberAreaWidth(0);
    }
}

inline void CustomTextEdit::setLineNumberAlignment(Qt::Alignment alignment) {
    lineNumberAlign = alignment;
    if (lineNumberArea) {
        lineNumberArea->update();
    }
}

inline void CustomTextEdit::setLineNumberMargin(int margin) {
    lineNumberMarginPx = margin;
    if (lineNumberArea) {
        lineNumberArea->update();
    }
}

inline int CustomTextEdit::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    QFontMetrics fm(lineNumberAreaFont);
    int space = lineNumberMarginPx * 2 + fm.horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

inline void CustomTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    
    // Fill background
    painter.fillRect(event->rect(), lineNumberBgColor);
    
    // Set font for line numbers
    painter.setFont(lineNumberAreaFont);
    QFontMetrics fm(lineNumberAreaFont);
    int lineHeight = fm.height();
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            
            // Highlight current line
            QTextCursor cursor = textCursor();
            if (cursor.blockNumber() == blockNumber) {
                painter.fillRect(0, top, lineNumberArea->width(), lineHeight, currentLineColor);
            }
            
            // Draw line number
            painter.setPen(lineNumberTextColor);
            QRect numberRect(0, top, lineNumberArea->width() - lineNumberMarginPx, lineHeight);
            painter.drawText(numberRect, lineNumberAlign | Qt::AlignVCenter, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

inline void CustomTextEdit::updateLineNumberAreaWidth(int newBlockCount) {
    Q_UNUSED(newBlockCount)
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

inline void CustomTextEdit::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy) {
        lineNumberArea->scroll(0, dy);
    } else {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }
    
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

inline void CustomTextEdit::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
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
    // Handle completer popup first
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
    
    // Handle special keys for auto-completion
    if (event->key() == Qt::Key_QuoteDbl && event->modifiers() == Qt::NoModifier) {
        autoDoubleStrings();
        event->accept();
        return;
    }
    else if (event->key() == Qt::Key_Apostrophe && event->modifiers() == Qt::NoModifier) {
        autoSingleStrings();
        event->accept();
        return;
    }
    
    // Handle other special keys
    bool keyHandled = false;
    
    switch (event->key()) {
        case Qt::Key_Tab:
            if (c && c->popup() && c->popup()->isVisible()) {
                c->popup()->hide();
                event->accept();
                return;
            }
            insertPlainText("    ");
            keyHandled = true;
            break;
        case Qt::Key_Backspace:
            handleBackspace();
            keyHandled = true;
            break;
        case Qt::Key_ParenLeft:
            autoParens();
            keyHandled = true;
            break;
        case Qt::Key_BraceLeft:
            autoBraces();
            keyHandled = true;
            break;
        case Qt::Key_BracketLeft:
            autoSquareBrackets();
            keyHandled = true;
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            handleEnter();
            keyHandled = true;
            break;
        default:
            break;
    }
    
    if (keyHandled) {
        event->accept();
        return;
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
                       completionPrefix.length() < 1 ||  
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
    if (completionPrefix.length() >= 1 && c->completionCount() > 0) { 
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

inline void CustomTextEdit::autoParens() {
    QTextCursor cursor = textCursor();
    cursor.insertText("()");
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);
}

inline void CustomTextEdit::autoDoubleStrings() {
    QTextCursor cursor = textCursor();
    
    // Проверяем, есть ли выделенный текст
    if (cursor.hasSelection()) {
        QString selectedText = cursor.selectedText();
        cursor.insertText("\"" + selectedText + "\"");
        
        // Устанавливаем курсор после закрывающей кавычки
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    } else {
        // Если нет выделения, вставляем пару кавычек и ставим курсор между ними
        cursor.insertText("\"\"");
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }
}

inline void CustomTextEdit::autoSingleStrings() {
    QTextCursor cursor = textCursor();
    
    // Проверяем, есть ли выделенный текст
    if (cursor.hasSelection()) {
        QString selectedText = cursor.selectedText();
        cursor.insertText("'" + selectedText + "'");
        
        // Устанавливаем курсор после закрывающей кавычки
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    } else {
        // Если нет выделения, вставляем пару кавычек и ставим курсор между ними
        cursor.insertText("''");
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }
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

inline void CustomTextEdit::createTips() {
    QStringList pythonKeyWord;
    
    pythonKeyWord 
        << "False" << "None" << "True" 
        << "and" << "as" << "assert" << "async" 
        << "await" << "break" << "class" << "continue" 
        << "def" << "del" << "elif" << "else" 
        << "except" << "finally" << "for" << "from" 
        << "global" << "if" << "import" << "in" 
        << "is" << "lambda" << "nonlocal" << "not" 
        << "or" << "pass" << "raise" << "return" 
        << "try" << "while" << "with" << "yield"
        
        // Специальные идентификаторы
        << "self" << "cls"
        
        // Встроенные функции и типы
        << "abs" << "all" << "any" << "ascii" 
        << "bin" << "bool" << "breakpoint" << "bytearray" 
        << "bytes" << "callable" << "chr" << "classmethod" 
        << "compile" << "complex" << "delattr" << "dict" 
        << "dir" << "divmod" << "enumerate" << "eval" 
        << "exec" << "filter" << "float" << "format" 
        << "frozenset" << "getattr" << "globals" << "hasattr" 
        << "hash" << "help" << "hex" << "id" 
        << "input" << "int" << "isinstance" << "issubclass" 
        << "iter" << "len" << "list" << "locals" 
        << "map" << "max" << "memoryview" << "min" 
        << "next" << "object" << "oct" << "open" 
        << "ord" << "pow" << "print" << "property" 
        << "range" << "repr" << "reversed" << "round" 
        << "set" << "setattr" << "slice" << "sorted" 
        << "staticmethod" << "str" << "sum" << "super" 
        << "tuple" << "type" << "vars" << "zip" 
        
        // Исключения
        << "BaseException" << "Exception" << "ArithmeticError" 
        << "BufferError" << "LookupError" << "AssertionError" 
        << "AttributeError" << "EOFError" << "FloatingPointError" 
        << "GeneratorExit" << "ImportError" << "ModuleNotFoundError" 
        << "IndexError" << "KeyError" << "KeyboardInterrupt" 
        << "MemoryError" << "NameError" << "NotImplementedError" 
        << "OSError" << "OverflowError" << "RecursionError" 
        << "ReferenceError" << "RuntimeError" << "StopIteration" 
        << "StopAsyncIteration" << "SyntaxError" << "IndentationError" 
        << "TabError" << "SystemError" << "SystemExit" 
        << "TypeError" << "UnboundLocalError" << "UnicodeError" 
        << "UnicodeDecodeError" << "UnicodeEncodeError" << "UnicodeTranslateError" 
        << "ValueError" << "ZeroDivisionError" << "EnvironmentError" 
        << "IOError" << "WindowsError" << "BlockingIOError" 
        << "ChildProcessError" << "ConnectionError" << "BrokenPipeError" 
        << "ConnectionAbortedError" << "ConnectionRefusedError" << "ConnectionResetError" 
        << "FileExistsError" << "FileNotFoundError" << "InterruptedError" 
        << "IsADirectoryError" << "NotADirectoryError" << "PermissionError" 
        << "ProcessLookupError" << "TimeoutError"
        
        // Константы
        << "__name__" << "__main__" << "__file__" << "__doc__"
        << "__package__" << "__version__";

    QCompleter *completer = new QCompleter(pythonKeyWord, this);
    setCompleter(completer);
}

#endif // CUSTOMTEXTEDIT_H