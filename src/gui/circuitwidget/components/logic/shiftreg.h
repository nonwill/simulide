/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SHIFTREG_H
#define SHIFTREG_H

#include "logiccomponent.h"

class LibraryItem;

class ShiftReg : public LogicComponent
{
    public:
        ShiftReg( QString type, QString id );
        ~ShiftReg();

        static Component* construct( QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }

        bool resetInv() { return m_resetInv; }
        void setResetInv( bool inv );

    private:
        bool m_resetInv;
};

#endif
