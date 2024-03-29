/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LATCHD_H
#define LATCHD_H

#include "logiccomponent.h"

class LibraryItem;

class LatchD : public LogicComponent
{
    public:
        LatchD( QString type, QString id );
        ~LatchD();

        static Component* construct( QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }

        int channels() { return m_channels; }
        void setChannels( int channels );

        virtual void setTristate( bool t ) override;
        virtual void setTrigger( trigger_t trigger ) override;

    private:
        void updateSize();
        
        int m_channels;
};

#endif

