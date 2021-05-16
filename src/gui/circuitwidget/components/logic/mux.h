/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#ifndef MUX_H
#define MUX_H

#include "logiccomponent.h"
#include "e-element.h"

class LibraryItem;

class MAINMODULE_EXPORT Mux : public LogicComponent, public eElement
{
    Q_OBJECT
    Q_PROPERTY( int  Address_Bits  READ addrBits   WRITE setAddrBits   DESIGNABLE true USER true )
    Q_PROPERTY( bool Invert_Inputs READ invertInps WRITE setInvertInps DESIGNABLE true USER true )
    Q_PROPERTY( bool tristate      READ tristate   USER true )

    public:
        QRectF boundingRect() const { return m_area; }
    
        Mux( QObject* parent, QString type, QString id );
        ~Mux();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }

        int addrBits() { return m_addrBits; }
        void setAddrBits( int bits );

        void setInvertInps( bool invert );

        bool tristate() { return true; }
        
        virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        int m_addrBits;

};

#endif

