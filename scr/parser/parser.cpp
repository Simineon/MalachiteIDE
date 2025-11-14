#include "parser.h"

Parser::Parser(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    // Настройка правил подсветки
    HighlightingRule rule;

    // Ключевые слова Python (синий)
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bprint\\b" << "\\bdef\\b" << "\\bclass\\b" << "\\bpass\\b"
                    << "\\breturn\\b" << "\\bFalse\\b" << "\\bTrue\\b" << "\\binput\\b"
                    << "\\bNone\\b" << "\\band\\b" << "\\bor\\b" << "\\bnot\\b"
                    << "\\bas\\b" << "\\bawait\\b" << "\\byield\\b" << "\\bcontinue\\b"
                    << "\\bdel\\b" << "\\bimport\\b" << "\\bfrom\\b" << "\\bin\\b"
                    << "\\bif\\b" << "\\belse\\b" << "\\belif\\b" << "\\bfor\\b"
                    << "\\bwhile\\b" << "\\bexcept\\b" << "\\btry\\b" << "\\brange\\b" 
                    << "\\bself\\b";

    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Строки (зеленый) ""
    doubleStringFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = doubleStringFormat;
    highlightingRules.append(rule);

    // Строки (зеленый) ''
    singleStringFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\'.*\'");
    rule.format = singleStringFormat;
    highlightingRules.append(rule);

    // Комментарии (серый)
    commentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // Числа (красный)
    numberFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("\\b\\d+\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void Parser::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}