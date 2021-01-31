// MLineEdit class - QLineEdit modified for passwords.
//
//   Copyright (C) 2021 by AK-47
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
// This file is part of the gazelle-installer.

#include <QApplication>
#include <QContextMenuEvent>
#include <QMenu>
#include <QStringList>
#include <QString>
#include "mlineedit.h"

MLineEdit::MLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
}

bool MLineEdit::isValid()
{
    return lastValid;
}

void MLineEdit::setup(MLineEdit *slave, int min, int genMin, int wordMax)
{
    this->slave = slave;
    this->min = min;
    this->genMin = genMin;
    this->wordMax = wordMax;
    disconnect(this);
    disconnect(slave);
    connect(this, &QLineEdit::textChanged, this, &MLineEdit::masterTextChanged);
    connect(slave, &QLineEdit::textChanged, this, &MLineEdit::slaveTextChanged);
    if(min==0) lastValid = true; // Control starts with no text
    generate(); // Pre-load the generator
}

void MLineEdit::generate()
{
    static QStringList words;
    static int pos;
    if(words.isEmpty()) {
        QFile file("/usr/share/dict/words");
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            while (!file.atEnd()) words.append(file.readLine().trimmed());
            file.close();
        } else {
            words << "tuls"<<"tihs"<<"yssup"<<"ssip"<<"kcuf"<<"gaf"<<"ehcuod"<<"kcid";
            words << "nmad"<<"tnuc"<<"parc"<<"kcoc"<<"hctib"<<"dratsab"<<"elohssa";
        }
        std::srand(unsigned(std::time(0)));
        pos = words.count();
    }
    genText.clear();
    do {
        if(pos>=words.count()) {
            std::random_shuffle(words.begin(), words.end());
            pos = 0;
        }
        const QString &word = words.at(pos);
        if(word.length() < wordMax) {
            genText.append(words.at(pos));
            genText.append('-');
        }
        ++pos;
    } while(genText.length() <= genMin);
    genText.append(QString::number(std::rand()%10));
}

void MLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    if(slave) {
        generate();
        menu->addSeparator();
        QAction *action = menu->addAction(tr("Use password") + ": " + genText);
        connect(action, &QAction::triggered, this, &MLineEdit::menuGenText);
    }
    menu->exec(event->globalPos());
    delete menu;
}

void MLineEdit::menuGenText()
{
    setText(genText);
    slave->setText(genText);
}

void MLineEdit::masterTextChanged()
{
    slave->clear();
    setPalette(QApplication::palette());
    slave->setPalette(QApplication::palette());
    const bool valid = (text().isEmpty() && min==0);
    if(valid != lastValid) {
        lastValid = valid;
        emit validationChanged(valid);
    }
}

void MLineEdit::slaveTextChanged(const QString &slaveText)
{
    QPalette pal = palette();
    bool valid = true;
    if (slaveText == text()) {
        QColor col(255, 255, 0, 40);
        if(slaveText.length()>=min) col.setRgb(0, 255, 0, 40);
        else valid = false;
        pal.setColor(QPalette::Base, col);
    } else {
        pal.setColor(QPalette::Base, QColor(255, 0, 0, 70));
        valid = false;
    }
    setPalette(pal);
    slave->setPalette(pal);
    if(valid != lastValid) {
        lastValid = valid;
        emit validationChanged(valid);
    }
}