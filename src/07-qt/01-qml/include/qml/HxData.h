#pragma once
/*
 * Copyright (C) 2025 Heng_Xin. All rights reserved.
 *
 * This file is part of HXTest.
 *
 * HXTest is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HXTest is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HXTest.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _HX_HX_DATA_H_
#define _HX_HX_DATA_H_

#include <QObject>
#include <QDebug>
#include <QVariant>
// #include <QtQml>

class HxData : public QObject {
    Q_OBJECT

    // 别用这个, 路径有Bug, 服了
    // QML_ELEMENT
public:
    explicit HxData(QObject* parnt = nullptr)
        : QObject(parnt)
    {}

    // 使用 Q_INVOKABLE 宏修饰的方法才可以在 QML 中被调用
    Q_INVOKABLE void fun() {
        emit cppSignal("C++ 真人出场!", 233);
        qDebug() << "debug: c++ run!";
    }

    // 单例演示
    static HxData* get() {
        static HxData res;
        return &res;
    }

    int getVal() const { return _val; }
    QString getStr() const { return _str; }

    void setVal(int val) { _val = val; }
    void setStr(QString const& str) { _str = str; }

Q_SIGNALS:
    void valChanged();
    void strChanged();
    void cppSignal(QVariant str, int x);

public Q_SLOTS:
    void cppSlots(QString const& str, int x) {
        _val += x;
        qDebug() << "cppSlots:" << str << x;
    }

private:
    int _val = 0;
    QString _str;

    Q_PROPERTY(
        int _val 
        READ getVal 
        WRITE setVal 
        NOTIFY valChanged
    )
    Q_PROPERTY(
        QString _str 
        READ getStr 
        WRITE setStr 
        NOTIFY strChanged
    )
};

#endif // !_HX_HX_DATA_H_