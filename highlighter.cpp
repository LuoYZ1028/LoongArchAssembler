#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // .开头标志
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::magenta);
    rule.pattern = QRegularExpression("\\.[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    // 关键字高亮设置
    keywordFormat.setForeground(Qt::green);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\badd\\.w\\b" << "\\bsub.w\\b" << "\\bslt\\b" << "\\bsltu\\b"
                    << "\\bnor\\b" << "\\band\\b" << "\\bor\\b" << "\\bxor\\b"
                    << "\\bsll.w\\b" << "\\bsrl.w\\b" << "\\bsra.w\\b" << "\\bmul.w\\b"
                    << "\\bmulh.w\\b" << "\\bmulh.wu\\b" << "\\bdiv.w\\b" << "\\bmod.w\\b"
                    << "\\bdiv.wu\\b" << "\\bmod.wu\\b" << "\\bbreak\\b" << "\\bsyscall\\b"

                    << "\\brdcntid.w\\b" << "\\brdcntvl.w\\b" << "\\brdcntvh.w\\b"

                    << "\\bslli.w\\b" << "\\bsrli.w\\b" << "\\bsrai.w\\b"

                    << "\\bslti\\b" << "\\bsltui\\b" << "\\baddi.w\\b" << "\\bandi\\b"
                    << "\\bori\\b" << "\\bxori\\b"

                    << "\\bld.b\\b" << "\\bld.h\\b" << "\\bld.w\\b" << "\\bst.b\\b"
                    << "\\bst.h\\b" << "\\bst.w\\b" << "\\bld.bu\\b" << "\\bld.hu\\b"
                    << "\\bpreld\\b"

                    << "\\bll.w\\b" << "\\bsc.w\\b"

                    << "\\bjirl\\b" << "\\bb\\b" << "\\bbl\\b" << "\\bbeq\\b"
                    << "\\bbne\\b" << "\\bblt\\b" << "\\bbge\\b" << "\\bbltu\\b"
                    << "\\bbgeu\\b"

                    << "\\blu12i.w\\b" << "\\bpcaddu12i\\b"

                    << "\\bdbar\\b" << "\\bibar\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 变量名+标号
    labelvarFormat.setFontWeight(QFont::Bold);
    labelvarFormat.setForeground(Qt::yellow);
    rule.pattern = QRegularExpression("[_A-Za-z]+:");
    rule.format = labelvarFormat;
    highlightingRules.append(rule);

    // 单行注释
    singleLineCommentFormat.setFontWeight(QFont::Bold);
    singleLineCommentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 多行注释
    multiLineCommentFormat.setForeground(Qt::gray);
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");

    // 引号及其内容
    quotationFormat.setForeground(Qt::cyan);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 16进制数
    functionFormat.setFontWeight(QFont::Bold);
    functionFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("\\b0x[0-9a-f]{1,8}\\b");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
            commentLength = endIndex - startIndex + match.capturedLength();
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
