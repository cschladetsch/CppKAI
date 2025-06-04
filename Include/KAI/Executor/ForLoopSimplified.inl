// Simplified ForLoop implementation for Executor
// This file should be included in ExecutorPerform.inl

case Operation::ForLoop: {
    KAI_TRACE() << "ForLoop: Starting execution";
    
    try {
        // Validate stack
        if (!data_.Valid() || !data_.Exists()) {
            KAI_TRACE_ERROR() << "ForLoop: Invalid data stack";
            break;
        }
        
        if (data_->Size() < 4) {
            KAI_TRACE_ERROR() << "ForLoop: Need at least 4 items on stack";
            break;
        }
        
        // Check the top of stack to determine syntax type
        auto top = data_->At(data_->Size() - 1);
        
        // SYNTAX 1: Range-based (Pi style)
        // Stack: accumulator start end body_continuation
        if (top.IsType<Continuation>() && 
            data_->At(data_->Size() - 2).IsType<int>() &&
            data_->At(data_->Size() - 3).IsType<int>()) {
            
            KAI_TRACE() << "ForLoop: Range-based syntax detected";
            
            auto body = Pop();
            int end = ConstDeref<int>(Pop());
            int start = ConstDeref<int>(Pop());
            Object accumulator = Pop();
            
            if (!body.IsType<Continuation>()) {
                KAI_TRACE_ERROR() << "ForLoop: Body must be continuation";
                Push(accumulator);
                break;
            }
            
            Pointer<Continuation> bodyCont = body;
            
            // Execute range loop
            for (int i = start; i <= end; ++i) {
                // Push accumulator and current value
                Push(accumulator);
                Push(New<int>(i));
                
                // Execute body inline
                if (bodyCont->GetCode().Exists()) {
                    for (int j = 0; j < bodyCont->GetCode()->Size(); ++j) {
                        if (break_ || continue_) break;
                        auto obj = bodyCont->GetCode()->At(j);
                        if (obj.Exists()) {
                            Eval(obj);
                        }
                    }
                }
                
                // Handle control flow
                if (break_) {
                    break_ = false;
                    break;
                }
                
                // Get new accumulator value
                if (!data_->Empty()) {
                    accumulator = Pop();
                }
            }
            
            // Push final accumulator
            Push(accumulator);
        }
        // SYNTAX 2: Traditional (C-style) 
        // Stack: init_cont cond_cont incr_cont body_cont
        else {
            KAI_TRACE() << "ForLoop: Traditional syntax detected";
            
            auto body = Pop();
            auto incr = Pop();
            auto cond = Pop();
            auto init = Pop();
            
            // Validate all are continuations
            if (!init.IsType<Continuation>() || !cond.IsType<Continuation>() ||
                !incr.IsType<Continuation>() || !body.IsType<Continuation>()) {
                KAI_TRACE_ERROR() << "ForLoop: All 4 items must be continuations";
                break;
            }
            
            Pointer<Continuation> initCont = init;
            Pointer<Continuation> condCont = cond;
            Pointer<Continuation> incrCont = incr;
            Pointer<Continuation> bodyCont = body;
            
            // Execute initialization
            ExecuteContinuationInline(initCont);
            
            // Main loop
            break_ = false;
            while (true) {
                continue_ = false;
                
                // Check condition
                ExecuteContinuationInline(condCont);
                
                if (data_->Empty() || !PopBool()) {
                    break;
                }
                
                // Execute body
                ExecuteContinuationInline(bodyCont);
                
                if (break_) {
                    break_ = false;
                    break;
                }
                
                // Execute increment (even with continue)
                ExecuteContinuationInline(incrCont);
            }
        }
    }
    catch (const Exception::Base& e) {
        KAI_TRACE_ERROR() << "ForLoop: " << e.ToString();
    }
    catch (const std::exception& e) {
        KAI_TRACE_ERROR() << "ForLoop: " << e.what();
    }
    
    break;
}

// Helper method to execute a continuation's code inline
void ExecuteContinuationInline(Pointer<Continuation> cont) {
    if (cont.Exists() && cont->GetCode().Exists()) {
        for (int i = 0; i < cont->GetCode()->Size(); ++i) {
            if (break_ || continue_) break;
            auto obj = cont->GetCode()->At(i);
            if (obj.Exists()) {
                Eval(obj);
            }
        }
    }
}