#ifndef PARSER_H
#define PARSER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class Parser : public QSyntaxHighlighter {
    Q_OBJECT

public:
    Parser(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat doubleStringFormat;
    QTextCharFormat singleStringFormat;
    QTextCharFormat multiLineStringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat decoratorFormat;

    void highlightMultiLine(const QString &text, const QString &delimiter, const QTextCharFormat &format);
};

#endif // PARSER_H