/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#ifndef DAC_H
#define DAC_H

#include "logiccomponent.h"
#include "e-element.h"

class LibraryItem;

class MAINMODULE_EXPORT DAC : public LogicComponent, public eElement
{
    Q_OBJECT
    Q_PROPERTY( double Vref         READ maxVolt    WRITE setMaxVolt    DESIGNABLE true USER true )
    Q_PROPERTY( int    Num_Bits     READ numInps    WRITE setNumInps    DESIGNABLE true USER true )

    public:
        DAC( QObject* parent, QString type, QString id );
        ~DAC();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();
        
        virtual QList<propGroup_t> propGroups() override;

        double maxVolt()               { return m_maxVolt; }
        void setMaxVolt( double volt ) { m_maxVolt = volt; }

        double maxValue()               { return m_maxValue; }
        void setMaxValue( double volt ) { m_maxValue = volt; }

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void setNumInps( int inputs );
        
    protected:
        double m_maxVolt;
        double m_maxValue;

        int m_value;
};

#endif
