// Basic partition editor for the installer.
//
//   Copyright (C) 2020 by AK-47
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

#include <QDebug>
#include <QLocale>
#include <QTreeWidget>
#include <QComboBox>
#include <QLineEdit>

#include "mparted.h"

MParted::MParted(MProcess &mproc, BlockDeviceList &bdlist, QObject *parent)
    : QObject(parent), proc(mproc), listBlkDevs(bdlist)
{

}

void MParted::setup(QTreeWidget *twParts)
{
    treePartitions = twParts;
    listUsePresets << "" << "/" << "/boot" << "/home" << "swap";
}

void MParted::populate()
{
    treePartitions->clear();
    QTreeWidgetItem *curdrv = nullptr;
    for (const BlockDeviceInfo &bdinfo : listBlkDevs) {
        QTreeWidgetItem *curdev;
        if (bdinfo.isDrive) {
            curdrv = curdev = new QTreeWidgetItem(treePartitions);
            curdrv->setData(0, Qt::UserRole, QVariant(false)); // drive starts off "unused"
            // Model
            curdev->setText(2, bdinfo.model);
        } else {
            curdev = new QTreeWidgetItem(curdrv);
            curdrv->setData(0, Qt::UserRole, QVariant(true)); // drive is now "used"
            curdev->setCheckState(4, Qt::Unchecked);
            // Label
            QLineEdit *editLabel = new QLineEdit(treePartitions);
            editLabel->setAutoFillBackground(true);
            treePartitions->setItemWidget(curdev, 2, editLabel);
            editLabel->setEnabled(false);
            editLabel->setText(bdinfo.label);
            // Use For
            QComboBox *comboUse = new QComboBox(treePartitions);
            comboUse->setAutoFillBackground(true);
            treePartitions->setItemWidget(curdev, 3, comboUse);
            comboUse->setEditable(true);
            comboUse->setInsertPolicy(QComboBox::NoInsert);
            comboUse->addItems(listUsePresets);
            comboUse->setProperty("row", QVariant::fromValue<void *>(curdev));
            connect(comboUse, &QComboBox::currentTextChanged, this, &MParted::comboUseTextChange);
            // Type
            QComboBox *comboType = new QComboBox(treePartitions);
            comboType->setAutoFillBackground(true);
            treePartitions->setItemWidget(curdev, 5, comboType);
            comboType->setEnabled(false);
            curdev->setText(5, bdinfo.fs);
            comboType->addItem(bdinfo.fs);
        }
        curdev->setText(0, bdinfo.name);
        curdev->setText(1, QLocale::system().formattedDataSize(bdinfo.size, 1, QLocale::DataSizeTraditionalFormat));
    }
    treePartitions->expandAll();
    for (int ixi = treePartitions->columnCount() - 1; ixi >= 0; --ixi) {
        if(ixi != 2) treePartitions->resizeColumnToContents(ixi);
    }
}

void MParted::comboUseTextChange(const QString &text)
{
    QComboBox *combo = static_cast<QComboBox *>(sender());
    if(!combo) return;
    int oldUseClass = combo->property("class").toInt();
    int useClass = -1;
    if(text.isEmpty()) useClass = 0;
    else if(text == "swap") useClass = 1;
    else if(text == "/boot") useClass = 2;
    else useClass = 3;
    if(useClass != oldUseClass) {
        QTreeWidgetItem *item = static_cast<QTreeWidgetItem *>(combo->property("row").value<void *>());
        if(!item) return;
        QComboBox *comboType = static_cast<QComboBox *>(treePartitions->itemWidget(item, 5));
        const QString &fs = item->text(5);
        comboType->setEnabled(false);
        comboType->clear();
        if(useClass == 0) comboType->addItem(fs);
        else if(useClass == 1) comboType->addItem("SWAP");
        else if(useClass == 2) comboType->addItem("ext4");
        else if(useClass == 3) {
            comboType->addItem("ext4");
            comboType->addItem("ext3");
            comboType->addItem("ext2");
            comboType->addItem("f2fs");
            comboType->addItem("jfs");
            comboType->addItem("xfs");
            comboType->addItem("btrfs");
            comboType->addItem("btrfs-zlib");
            comboType->addItem("btrfs-lzo");
            comboType->addItem("reiserfs");
            comboType->addItem("reiser4");
            comboType->setEnabled(true);
        }
        treePartitions->itemWidget(item, 2)->setDisabled(text.isEmpty());
        combo->setProperty("class", QVariant(useClass));
    }
}
