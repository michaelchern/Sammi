#pragma once

//#include "runtime/core/math/vector2.h"

#include <memory>

namespace Sammi
{
    //class EditorUI;
    class SammiEngine;

    class SammiEditor 
    {
        friend class EditorUI;

    public:
        SammiEditor();
        virtual ~SammiEditor();

        void initialize(SammiEngine* engine_runtime);
        void clear();

        void run();

    protected:
        //std::shared_ptr<EditorUI> m_editor_ui;
        SammiEngine* m_engine_runtime{ nullptr };
    };
}
