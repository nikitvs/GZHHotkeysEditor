#include "creatorwidget.hpp"
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QPushButton>
#include <QVBoxLayout>

CreatorWidget::CreatorWidget(QWidget *parent)
	: QDialog(parent)
{
	// configure game buttons
	QRadioButton* generalsButton = new QRadioButton(Config::gameEnumToString(Config::Games::GENERALS));
	QRadioButton* zeroHourButton = new QRadioButton(Config::gameEnumToString(Config::Games::GENERALS_ZERO_HOUR));
	groupB.setExclusive(true);
	generalsButton->setChecked(true);
	groupB.addButton(generalsButton);
	groupB.addButton(zeroHourButton);
	QVBoxLayout* choiseL = new QVBoxLayout;
	choiseL->addWidget(generalsButton);
	choiseL->addWidget(zeroHourButton);

	// configure save option
	saveToGameBox.setText(tr("Save hotkey dirrectly to the game"));

	// configure dialog buttons
	QDialogButtonBox* dialogBB = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	dialogBB->button(QDialogButtonBox::Ok)->setText(tr("Start configurating"));
	dialogBB->button(QDialogButtonBox::Cancel)->setText(tr("Back"));
	// emit accepted configurations
	connect(dialogBB, &QDialogButtonBox::accepted, this,
		[=](){
			emit acceptedConfiguration(static_cast<Config::Games>(groupB.checkedId()),
																  saveToGameBox.isChecked());
		;}
	);
	connect(dialogBB, &QDialogButtonBox::rejected, this, &QDialog::deleteLater);

	// configure dialog view
	QVBoxLayout* mainL = new QVBoxLayout;
	mainL->setAlignment(Qt::Alignment::enum_type::AlignCenter);
	mainL->addStretch(5);
	mainL->addLayout(choiseL);
	mainL->addStretch(2);
	mainL->addWidget(&saveToGameBox);
	mainL->addStretch(5);
	mainL->addWidget(dialogBB, 0, Qt::AlignCenter);
	mainL->addStretch(1);
	for(auto & button : dialogBB->buttons())
		button->setStyleSheet("QPushButton { padding-left: 30px; padding-right: 30px; }");
	setLayout(mainL);
}