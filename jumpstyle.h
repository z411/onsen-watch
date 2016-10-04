#ifndef JUMPSTYLE_H
#define JUMPSTYLE_H

#include <QProxyStyle>


class JumpStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const;
};

#endif // JUMPSTYLE_H
