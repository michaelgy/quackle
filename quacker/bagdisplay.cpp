/*
 *  Quackle -- Crossword game artificial intelligence and analysis tool
 *  Copyright (C) 2005-2019 Jason Katz-Brown, John O'Laughlin, and John Fultz.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>

#include <QtWidgets>

#include <bag.h>
#include <game.h>
#include <quackleio/util.h>

#include "bagdisplay.h"
#include "geometry.h"

static QTextEdit *createTileTextEdit()
{
	QTextEdit *textEdit = new QTextEdit;
	textEdit->setReadOnly(true);
	textEdit->setFontFamily("Courier");
	textEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	return textEdit;
}

static QWidget *createTab(QLabel **outLabel, QTextEdit **outTextEdit)
{
	QWidget *tab = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout(tab);
	Geometry::setupInnerLayout(layout);

	*outTextEdit = createTileTextEdit();

	*outLabel = new QLabel;
	(*outLabel)->setWordWrap(true);
	(*outLabel)->setBuddy(*outTextEdit);
	layout->addWidget(*outLabel);
	layout->addWidget(*outTextEdit);

	layout->setStretchFactor(*outTextEdit, 10);

	return tab;
}

BagDisplay::BagDisplay(QWidget *parent)
	: View(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	Geometry::setupInnerLayout(layout);

	m_tabWidget = new QTabWidget;
	layout->addWidget(m_tabWidget);

	QWidget *unseenTab = createTab(&m_unseenLabel, &m_unseenTextEdit);
	m_tabWidget->addTab(unseenTab, tr("Unseen"));

	QWidget *bagTab = createTab(&m_bagLabel, &m_bagTextEdit);
	m_tabWidget->addTab(bagTab, tr("Bag"));

	formatTiles(Quackle::LongLetterString(), m_unseenLabel, m_unseenTextEdit,
		QString(), tr("&Bag is collapsed in a wrinkled heap on the table"));
	formatTiles(Quackle::LongLetterString(), m_bagLabel, m_bagTextEdit,
		QString(), tr("Bag is empty"));
}

BagDisplay::~BagDisplay()
{
}

void BagDisplay::positionChanged(const Quackle::GamePosition *position)
{
	formatTiles(position->unseenBag().tiles(), m_unseenLabel, m_unseenTextEdit,
		tr("%1 unseen tiles"), tr("&Bag is collapsed in a wrinkled heap on the table"));
	formatTiles(position->bag().tiles(), m_bagLabel, m_bagTextEdit,
		tr("%1 tiles in bag"), tr("Bag is empty"));

	// Birthday
	for (const auto& it : position->players())
	{
		if (it.name() == "zorbonauts")
		{
			m_unseenLabel->setText(tr("The bag is collapsed in a transparent dead jellyfish-like heap on the table while flies buzz round"));
			break;
		}
	}
}

void BagDisplay::formatTiles(const Quackle::LongLetterString &tiles, QLabel *label, QTextEdit *textEdit, const QString &labelFormat, const QString &emptyMessage)
{
	if (tiles.empty())
	{
		label->setText(emptyMessage);
		textEdit->clear();
		return;
	}

	QMap<Quackle::Letter, int> counts;
	QString text;

	for (Quackle::LongLetterString::const_iterator it = tiles.begin(); it != tiles.end(); ++it)
	{
		if (counts.contains(*it))
			counts[*it] += 1;
		else
			counts.insert(*it, 1);
	}

	QFontMetrics metrics(textEdit->currentFont());
	int maxLineWidth = 0;

	for (QMap<Quackle::Letter, int>::iterator it = counts.begin(); it != counts.end(); ++it)
	{
		const int count = it.value();

		QString line;

		const QString qstring = QuackleIO::Util::letterToQString(it.key());
		const QString sanitizedQString = QuackleIO::Util::sanitizeUserVisibleLetterString(qstring);
		const bool separateWithSpaces = qstring != sanitizedQString;
		for (int i = 0; i < count; ++i)
		{
			if (separateWithSpaces && i > 0) line += " ";
			line += sanitizedQString;
		}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
		const int lineWidth = metrics.horizontalAdvance(line);
#else
		const int lineWidth = metrics.width(line);
#endif
		if (lineWidth > maxLineWidth)
			maxLineWidth = lineWidth;

		text += line;
		text += "\n";
	}

	label->setText(labelFormat.arg(tiles.length()));
	textEdit->setPlainText(text);

	const int minimumMaxLineWidth = 16;
	if (maxLineWidth < minimumMaxLineWidth)
		maxLineWidth = minimumMaxLineWidth;

	const int maximumWidth = maxLineWidth + textEdit->frameWidth() * 2 + (textEdit->verticalScrollBar()->isVisible()? textEdit->verticalScrollBar()->width() : 0) + 10;
	textEdit->setMaximumSize(maximumWidth, 26 * 100);

	textEdit->resize(textEdit->maximumSize());
}
