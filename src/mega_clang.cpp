// Copyright 2017-2018 Todd Fleming
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <stdio.h>

#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/FrontendTool/Utils.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/TargetSelect.h"

static void LLVMErrorHandler(void *user_data, const std::string &Message, bool GenCrashDiag)
{
    clang::DiagnosticsEngine &Diags = *static_cast<clang::DiagnosticsEngine *>(user_data);
    Diags.Report(clang::diag::err_fe_error_backend) << Message;
    llvm::sys::RunInterruptHandlers();
    llvm::sys::Process::Exit(GenCrashDiag ? 70 : 1);
}

extern "C" bool clang_compile(
        const char *args[],
        size_t n_args,
        char **diags_out)
{
    if (args == NULL || diags_out == NULL) {
        assert(0);
        return false;
    }

    printf("args:\n");
    for (size_t i = 0; i < n_args; i++) {
        printf("  %s\n", args[i]);
    }

    std::unique_ptr<clang::CompilerInstance> Clang(new clang::CompilerInstance());
    clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> diag_id(new clang::DiagnosticIDs());

    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();

    std::string diags_str;
    llvm::raw_string_ostream diags_stream(diags_str);

    clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions();
    clang::TextDiagnosticPrinter *diags_printer = new clang::TextDiagnosticPrinter(diags_stream, DiagOpts.get());
    clang::DiagnosticsEngine diags(diag_id, &*DiagOpts, diags_printer);

    bool success = clang::CompilerInvocation::CreateFromArgs(Clang->getInvocation(), llvm::makeArrayRef(args, n_args), diags);
    Clang->createDiagnostics();
    if (!Clang->hasDiagnostics()) {
        *diags_out = strdup("does not have diagnostics");
        return false;
    }

    llvm::install_fatal_error_handler(LLVMErrorHandler,
                                      static_cast<void *>(&Clang->getDiagnostics()));

    for (auto &input : Clang->getFrontendOpts().Inputs) {
        std::cout << "input: '" << input.getFile().data() << "'" << std::endl;
    }

    std::cout << "output: '" << Clang->getFrontendOpts().OutputFile.data() << "'" << std::endl;
    if (!success) {
        *diags_out = strdup(diags_str.data());
        return false;
    }

    success = ExecuteCompilerInvocation(Clang.get());
    if (!success) {
        *diags_out = strdup(diags_str.data());
        return false;
    }

    llvm::remove_fatal_error_handler();
    return true;
}
