/*
    A straightforward implementation of a Shadow Stack according to ROPdefender: A Detection
    Tool to Defend Against Return-Oriented Programming Attacks by Lucas Davi, Ahmad-Reza
    Sadeghi, and Marcel Winandy.

    The current implementation cannot correctly instrument programs utilising the Unix signals API.
*/
#include "pin.H"

#include <iostream>
#include <stack>

#define DEBUG 0

using std::stack;
using std::cerr; using std::endl; using std::hex;

unsigned int lastThread = 42;
stack<ADDRINT> *shadow = NULL;

// Key for accessing TLS storage in the threads. initialized once in main()
static TLS_KEY tlsKey;

VOID updateCurrentThreadStack(const THREADID threadId) {
    // If no context switch happened between two bbls, no need to fetch the shadow again
    if (threadId != lastThread) {
        // Get thread-specific data
        shadow = static_cast<stack<ADDRINT> *>(PIN_GetThreadData(tlsKey, threadId));
        lastThread = threadId;
    }
}

VOID onCall(const ADDRINT callInsAddr, const USIZE size, const THREADID threadId) {
    updateCurrentThreadStack(threadId);
    ADDRINT nextInstAddr = callInsAddr + size;

#if DEBUG
    cerr << "Push on stack: 0x" << hex << nextInstAddr << endl;
#endif
    shadow->push(nextInstAddr);
}

VOID onRet(const ADDRINT retAddr, THREADID threadId) {
    updateCurrentThreadStack(threadId);
    ADDRINT saved = 0;

    while (!shadow->empty()) {
        saved = shadow->top();
        shadow->pop();
#if DEBUG
    cerr << "poped value from stack: 0x" << hex << saved << endl;
    cerr << "return value is: 0x" << hex << retAddr << endl;
#endif
        if (saved == retAddr) { return; }
    }

    cerr << "return address not equal to call address! (0x"
         << hex << max(saved, retAddr) - min(saved, retAddr) << ")" << endl;
    exit(1);
}

VOID ThreadStart(THREADID threadId, CONTEXT *ctxt, INT32 flags, VOID *v) {
    stack<ADDRINT> *shadow = new stack<ADDRINT>();
    PIN_SetThreadData(tlsKey, shadow, threadId);
}

VOID ThreadEnd(THREADID threadId, const CONTEXT *ctxt, INT32 code, VOID *v) {
    updateCurrentThreadStack(threadId);
    delete(shadow);
}

VOID doTrace(TRACE tr, void*) {
    auto tail = BBL_InsTail(TRACE_BblTail(tr));
    if (INS_IsCall(tail)) {
        INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(&onCall), IARG_INST_PTR, IARG_UINT64, INS_Size(tail), IARG_THREAD_ID, IARG_END);
    } else if (INS_IsRet(tail)) {
        INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(&onRet), IARG_BRANCH_TARGET_ADDR, IARG_THREAD_ID, IARG_END);
    }
}

int main(int argc, char *argv[]) {
    // Initialize PIN library
    if (PIN_Init(argc,argv)) {
        cerr << "PIN Initialization failed!" << endl;
    }

    PIN_InitSymbols();

    // Obtain  a key for TLS storage.
    tlsKey = PIN_CreateThreadDataKey(0);
    TRACE_AddInstrumentFunction(doTrace, 0);

    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadEnd, 0);

    PIN_StartProgram();

    return 0;
}
