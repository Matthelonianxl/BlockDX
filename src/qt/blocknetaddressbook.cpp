// Copyright (c) 2018 The Blocknet Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blocknetaddressbook.h"
#include "blockneticonaltbtn.h"
#include "blocknetlabelbtn.h"
#include "blocknetavatar.h"

#include <QHeaderView>
#include <QApplication>

BlocknetAddressBook::BlocknetAddressBook(QWidget *popup, QFrame *parent) : QFrame(parent), popupWidget(popup), layout(new QVBoxLayout) {
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->setContentsMargins(46, 10, 50, 0);
    this->setLayout(layout);

    titleLbl = new QLabel(tr("Address Book"));
    titleLbl->setObjectName("h4");
    titleLbl->setFixedHeight(26);

    auto *topBox = new QFrame;
    auto *topBoxLayout = new QHBoxLayout;
    topBoxLayout->setContentsMargins(QMargins());
    topBox->setLayout(topBoxLayout);

    auto *addAddressBtn = new BlocknetIconAltBtn(":/redesign/QuickActions/AddressButtonIcon.png", 5);

    addButtonLbl = new QLabel(tr("Add Address"));
    addButtonLbl->setObjectName("h4");

    filterLbl = new QLabel(tr("Filter by:"));
    filterLbl->setObjectName("title");

    addressDropdown = new BlocknetDropdown;
    addressDropdown->addItem(tr("All Addresses"), FILTER_ALL);
    addressDropdown->addItem(tr("Sending"),      FILTER_SENDING);
    addressDropdown->addItem(tr("Receiving"),  FILTER_RECEIVING);

    addressTbl = new BlocknetAddressBookTable(popup);
    addressTbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addressTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    addressTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    addressTbl->setSelectionMode(QAbstractItemView::ExtendedSelection);
    addressTbl->setAlternatingRowColors(true);
    addressTbl->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    addressTbl->setShowGrid(false);
    addressTbl->setContextMenuPolicy(Qt::CustomContextMenu);
    addressTbl->setSortingEnabled(true);
    addressTbl->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    addressTbl->verticalHeader()->setDefaultSectionSize(78);
    addressTbl->verticalHeader()->setVisible(false);
    addressTbl->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    addressTbl->horizontalHeader()->setSortIndicatorShown(true);
    addressTbl->horizontalHeader()->setSectionsClickable(true);
    addressTbl->setColumnWidth(COLUMN_ACTION, 50);
    addressTbl->setColumnWidth(COLUMN_AVATAR, 50);

    topBoxLayout->addWidget(addAddressBtn, Qt::AlignLeft);
    topBoxLayout->addWidget(addButtonLbl, Qt::AlignLeft);
    topBoxLayout->addStretch(1);
    topBoxLayout->addWidget(filterLbl);
    topBoxLayout->addWidget(addressDropdown);

    layout->addWidget(titleLbl);
    layout->addSpacing(10);
    layout->addWidget(topBox);
    layout->addSpacing(15);
    layout->addWidget(addressTbl);
    layout->addSpacing(20);

    fundsMenu = new BlocknetFundsMenu;
    fundsMenu->setDisplayWidget(popupWidget);
    fundsMenu->hOnSendFunds = [&]() { emit sendFunds(); };
    fundsMenu->hOnRequestFunds = [&]() { emit requestFunds(); };
    fundsMenu->hide();
}

void BlocknetAddressBook::setWalletModel(WalletModel *w) {
    if (walletModel == w)
        return;

    walletModel = w;
    if (!walletModel || !walletModel->getOptionsModel())
        return;

    addressTbl->setWalletModel(walletModel);
    addressTbl->horizontalHeader()->setSectionResizeMode(BlocknetAddressBookFilterProxy::AddressBookAvatar, QHeaderView::Fixed);
    addressTbl->horizontalHeader()->setSectionResizeMode(BlocknetAddressBookFilterProxy::AddressBookAlias, QHeaderView::Stretch);
    addressTbl->horizontalHeader()->setSectionResizeMode(BlocknetAddressBookFilterProxy::AddressBookAddress, QHeaderView::ResizeToContents);
    addressTbl->horizontalHeader()->setSectionResizeMode(BlocknetAddressBookFilterProxy::AddressBookCopy, QHeaderView::Stretch);
    addressTbl->horizontalHeader()->setSectionResizeMode(BlocknetAddressBookFilterProxy::AddressBookEdit, QHeaderView::Stretch);
    addressTbl->horizontalHeader()->setSectionResizeMode(BlocknetAddressBookFilterProxy::AddressBookDelete, QHeaderView::Stretch);
}

void BlocknetAddressBook::unwatch() {
    addressTbl->setEnabled(false);
}

void BlocknetAddressBook::watch() {
    addressTbl->setEnabled(true);
}

void BlocknetAddressBook::aliasChanged(const QString &alias) {
    addressTbl->setAlias(alias);
}

void BlocknetAddressBook::addressChanged(const QString &address) {
    addressTbl->setAddress(address);
}

QVector<BlocknetAddressBook::Address> BlocknetAddressBook::filtered(int filter, int chainHeight) {
    QVector<Address> r;
    for (auto &d : dataModel) {
        switch (filter) {
            case FILTER_SENDING: 
            case FILTER_RECEIVING: 
            case FILTER_ALL:
            default:
                r.push_back(d);
                break;
        }
    }
    return r;
}

void BlocknetAddressBook::setData(QVector<Address> data) {
    this->filteredData = data;

    for (int i = 0; i < this->filteredData.count(); ++i) {
        auto &d = this->filteredData[i];

        // action item
        auto *widget = new QWidget();
        widget->setContentsMargins(QMargins());
        auto *boxLayout = new QVBoxLayout;
        boxLayout->setContentsMargins(QMargins());
        boxLayout->setSpacing(0);
        widget->setLayout(boxLayout);

        auto *button = new BlocknetActionBtn;
        button->setID(d.address);
        boxLayout->addWidget(button, 0, Qt::AlignCenter);
        connect(button, &BlocknetActionBtn::clicked, this, &BlocknetAddressBook::onAddressAction);

        // avatar
        auto *avatarWidget = new QWidget();
        avatarWidget->setContentsMargins(QMargins());
        auto *avatarLayout = new QVBoxLayout;
        avatarLayout->setContentsMargins(QMargins());
        avatarLayout->setSpacing(0);
        avatarWidget->setLayout(avatarLayout);

        auto *avatar = new BlocknetAvatar(d.alias);
        avatarLayout->addWidget(avatar, 0, Qt::AlignCenter);

        // copy item
        auto *copyWidget = new QWidget();
        copyWidget->setContentsMargins(QMargins());
        auto *copyLayout = new QVBoxLayout;
        copyLayout->setContentsMargins(QMargins());
        copyLayout->setSpacing(0);
        copyWidget->setLayout(copyLayout);

        auto *copyButton = new BlocknetLabelBtn;
        copyButton->setText(tr("Copy Address"));
        //copyButton->setFixedSize(40, 40);
        copyButton->setID(d.address);
        copyLayout->addWidget(copyButton, 0, Qt::AlignCenter);
        copyLayout->addSpacing(6);
        //connect(copyButton, &BlocknetLabelBtn::clicked, this, &BlocknetAddressBook::onAddressAction);

        // edit item
        auto *editWidget = new QWidget();
        editWidget->setContentsMargins(QMargins());
        auto *editLayout = new QVBoxLayout;
        editLayout->setContentsMargins(QMargins());
        editLayout->setSpacing(0);
        editWidget->setLayout(editLayout);

        auto *editButton = new BlocknetLabelBtn;
        editButton->setText(tr("Edit"));
        //editButton->setFixedSize(40, 40);
        editButton->setID(d.address);
        editLayout->addWidget(editButton, 0, Qt::AlignCenter);
        editLayout->addSpacing(6);
        //connect(editButton, &BlocknetLabelBtn::clicked, this, &BlocknetAddressBook::onAddressAction);

        // delete item
        auto *deleteWidget = new QWidget();
        deleteWidget->setContentsMargins(QMargins());
        auto *deleteLayout = new QVBoxLayout;
        deleteLayout->setContentsMargins(QMargins());
        deleteLayout->setSpacing(0);
        deleteWidget->setLayout(deleteLayout);

        auto *deleteButton = new BlocknetLabelBtn;
        deleteButton->setText(tr("Delete"));
        //deleteButton->setFixedSize(40, 40);
        deleteButton->setID(d.address);
        deleteLayout->addWidget(deleteButton, 0, Qt::AlignCenter);
        deleteLayout->addSpacing(6);
        //connect(deleteButton, &BlocknetLabelBtn::clicked, this, &BlocknetAddressBook::onAddressAction);
    }
}

void BlocknetAddressBook::onAddressAction() {
    auto *btn = qobject_cast<BlocknetActionBtn*>(sender());
    auto addressHash = uint256S(btn->getID().toStdString());
    if (fundsMenu->isHidden()) {
        QPoint li = btn->mapToGlobal(QPoint());
        QPoint npos = popupWidget->mapFromGlobal(QPoint(li.x() - 2, li.y() + btn->height() + 2));
        fundsMenu->move(npos);
        fundsMenu->show();
    }
}

BlocknetAddressBookTable::BlocknetAddressBookTable(QWidget *parent) : QTableView(parent),
                                                                                    walletModel(nullptr) {
}

void BlocknetAddressBookTable::setWalletModel(WalletModel *w) {
    if (walletModel == w)
        return;
    walletModel = w;

    if (walletModel == nullptr) {
        setModel(nullptr);
        return;
    }

    //this->setItemDelegateForColumn(BlocknetAddressBookFilterProxy::AddressBookAction, new BlocknetAddressBookCellItem(this));

    // Set up transaction list
    auto *filter = new BlocknetAddressBookFilterProxy(walletModel->getOptionsModel(), this);
    filter->setSourceModel(walletModel->getAddressTableModel());
    filter->setDynamicSortFilter(true);
    filter->setSortRole(Qt::EditRole);
    filter->setFilterRole(Qt::EditRole);
    filter->setSortCaseSensitivity(Qt::CaseInsensitive);
    filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filter->sort(BlocknetAddressBookFilterProxy::AddressBookAlias, Qt::DescendingOrder);
    setModel(filter);
}

void BlocknetAddressBookTable::leave() {
    this->blockSignals(true);
    model()->blockSignals(true);
}
void BlocknetAddressBookTable::enter() {
    this->blockSignals(false);
    model()->blockSignals(false);
}

void BlocknetAddressBookTable::setAlias(const QString &alias) {
    auto *m = dynamic_cast<BlocknetAddressBookFilterProxy*>(this->model());
    m->setAlias(alias);
}

void BlocknetAddressBookTable::setAddress(const QString &address) {
    auto *m = dynamic_cast<BlocknetAddressBookFilterProxy*>(this->model());
    m->setAddress(address);
}

BlocknetAddressBookFilterProxy::BlocknetAddressBookFilterProxy(OptionsModel *o, QObject *parent) : QSortFilterProxyModel(parent),
                                                                                                                 optionsModel(o),
                                                                                                                 limitRows(-1),
                                                                                                                 alias(QString()),
                                                                                                                 address(QString()) {}

void BlocknetAddressBookFilterProxy::setAlias(const QString &alias) {
    this->alias = alias;
    invalidateFilter();
}

void BlocknetAddressBookFilterProxy::setAddress(const QString &address) {
    this->address = address;
    invalidateFilter();
}

bool BlocknetAddressBookFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    return true;
}

bool BlocknetAddressBookFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    switch (left.column()) {
        case AddressBookAvatar: {
            auto l1 = sourceModel()->index(left.row(), AddressTableModel::Label).data(Qt::DisplayRole);
            auto r1 = sourceModel()->index(right.row(), AddressTableModel::Label).data(Qt::DisplayRole);
            return l1.toLongLong() < r1.toLongLong();
        }
        case AddressBookAlias: {
            auto l2 = sourceModel()->index(left.row(), AddressTableModel::Label).data(Qt::DisplayRole);
            auto r2 = sourceModel()->index(right.row(), AddressTableModel::Label).data(Qt::DisplayRole);
            return l2.toString() < r2.toString();
        }
        case AddressBookAddress: {
            auto l3 = sourceModel()->index(left.row(), AddressTableModel::Address).data(Qt::DisplayRole);
            auto r3 = sourceModel()->index(right.row(), AddressTableModel::Address).data(Qt::DisplayRole);
            return l3.toString() < r3.toString();
        }
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

int BlocknetAddressBookFilterProxy::columnCount(const QModelIndex &) const {
    return BlocknetAddressBookFilterProxy::AddressBookDelete + 1;
}

int BlocknetAddressBookFilterProxy::rowCount(const QModelIndex& parent) const {
    if (limitRows != -1)
        return std::min(QSortFilterProxyModel::rowCount(parent), limitRows);
    else
        return QSortFilterProxyModel::rowCount(parent);
}

QVariant BlocknetAddressBookFilterProxy::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
                case AddressBookAction:
                    return tr("");
                case AddressBookAvatar:
                    return tr("");
                case AddressBookAlias:
                    return tr("Alias");
                case AddressBookAddress:
                    return tr("Address");
                case AddressBookCopy:
                    return tr("");
                case AddressBookEdit:
                    return tr("");
                case AddressBookDelete:
                    return tr("");
                default:
                    return tr("");
            }
        } else if (role == Qt::TextAlignmentRole) {
            return Qt::AlignLeft;
        }
    }
    return QVariant();
}

QModelIndex BlocknetAddressBookFilterProxy::index(int row, int column, const QModelIndex &parent) const {
    if (column > 1)
        return createIndex(row, column);

    return QSortFilterProxyModel::index(row, column);
}

QModelIndex BlocknetAddressBookFilterProxy::parent(const QModelIndex &index) const {
    if (index.column() > 1)
        return QModelIndex();

    return QSortFilterProxyModel::parent(index);
}

QModelIndex BlocknetAddressBookFilterProxy::mapToSource(const QModelIndex &proxyIndex) const {
    auto index = proxyIndex;
    if (proxyIndex.column() > 1)
        index = this->index(proxyIndex.row(), 0);
    return QSortFilterProxyModel::mapToSource(index);
}

QVariant BlocknetAddressBookFilterProxy::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto *model = dynamic_cast<AddressTableModel*>(sourceModel());
    QModelIndex sourceIndex = mapToSource(index);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case AddressBookAction:
                return tr("");
            case AddressBookAvatar:
                return tr("");
            case AddressBookAlias:
                return model->data(model->index(sourceIndex.row(), 0, sourceIndex.parent()), role);
            case AddressBookAddress:
                return model->data(model->index(sourceIndex.row(), 1, sourceIndex.parent()), role);
            case AddressBookCopy:
                return tr("");
            case AddressBookEdit:
                return tr("");
            case AddressBookDelete:
                return tr("");
        }
    }
    return QVariant();
}

BlocknetAddressBookCellItem::BlocknetAddressBookCellItem(QObject *parent) : QStyledItemDelegate(parent) { }

void BlocknetAddressBookCellItem::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    /*if (index.column() == BlocknetAddressBookFilterProxy::AddressBookAction) {
        //painter->save();

        auto *widget = new QWidget();
        widget->setContentsMargins(QMargins());
        auto *boxLayout = new QVBoxLayout;
        boxLayout->setContentsMargins(QMargins());
        boxLayout->setSpacing(0);
        widget->setLayout(boxLayout);

        auto *button = new BlocknetActionBtn;
        //auto *address = static_cast<QString>(index.data(Qt::DisplayRole));
        //button->setID(address);
        boxLayout->addWidget(button, 0, Qt::AlignCenter);
        //connect(button, &BlocknetActionBtn::clicked, this, &BlocknetAddressBook::onAddressAction);

        QStyleOptionComplex box;
        box.rect = option.rect;

        //QWidget *srcWidget = qobject_cast<QWidget *>(option.styleObject);
        //// style->metaObject()->className() = QStyleSheetStyle
        //QStyle *style = srcWidget ? srcWidget->style() : QApplication::style();

        //style->drawComplexControl(QStyle::CC_CustomBase, &box, painter, widget);
        QApplication::style()->drawComplexControl(QStyle::CC_CustomBase, &box, painter, widget);
        //QApplication::style()->drawComplexControl(QStyle::CC)
        //QApplication::style()->drawControl(QStyle::CE_ComboBoxLabel, &box, painter, 0);
    
        //painter->restore();
    } else {
         QStyledItemDelegate::paint(painter, option, index);
    }*/
    //QStyledItemDelegate::paint(painter, option, index);
}

QWidget *BlocknetAddressBookCellItem::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {

    return QStyledItemDelegate::createEditor(parent, option, index);

    /*if (index.column() == BlocknetAddressBookFilterProxy::AddressBookAction) {
        auto *widget = new QWidget();
        widget->setContentsMargins(QMargins());
        auto *boxLayout = new QVBoxLayout;
        boxLayout->setContentsMargins(QMargins());
        boxLayout->setSpacing(0);
        widget->setLayout(boxLayout);

        auto *button = new BlocknetActionBtn;
        //auto *address = static_cast<QString>(index.data(Qt::DisplayRole));
        //button->setID(address);
        boxLayout->addWidget(button, 0, Qt::AlignCenter);
        //connect(button, &BlocknetActionBtn::clicked, this, &BlocknetAddressBook::onAddressAction);
        return widget;
    } else {
        auto *widget = new QWidget();
        widget->setContentsMargins(QMargins());
        auto *boxLayout = new QVBoxLayout;
        boxLayout->setContentsMargins(QMargins());
        boxLayout->setSpacing(0);
        widget->setLayout(boxLayout);

        auto *button = new BlocknetActionBtn;
        //auto *address = static_cast<QString>(index.data(Qt::DisplayRole));
        //button->setID(address);
        boxLayout->addWidget(button, 0, Qt::AlignCenter);
        //connect(button, &BlocknetActionBtn::clicked, this, &BlocknetAddressBook::onAddressAction);
        return widget;
        //return QStyledItemDelegate::createEditor(parent, option, index);
    }*/
}

QSize BlocknetAddressBookCellItem::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.column() == BlocknetAddressBookFilterProxy::AddressBookAction)
        return {50, option.rect.height()};
    return QStyledItemDelegate::sizeHint(option, index);
}
