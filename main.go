package main

// #cgo LDFLAGS: build/libmega_clang.a -lc++
// #include "src/mega_clang.h"
import "C"
import (
	"fmt"
	"unsafe"
)

func main() {
	result := compile([]string{
		"-emit-obj",
		"-o", "program.o",
		"test/program.c",
	})

	fmt.Println("success: ", result.success)
	fmt.Println("diags: ", result.diags)
}

type CompileResult struct {
	success bool
	diags   string
}

// args are for the clang frontend: clang -cc1 --help
func compile(args []string) CompileResult {
	argv := (**C.char)(C.malloc(C.ulong((len(args) + 1)) * C.ulong(unsafe.Sizeof((*C.char)(nil)))))
	defer C.free(unsafe.Pointer(argv))

	for i, _ := range args {
		elem := (**C.char)(unsafe.Pointer(uintptr(unsafe.Pointer(argv)) + uintptr(i)*unsafe.Sizeof(*argv)))
		*elem = C.CString(args[i])

		defer C.free(unsafe.Pointer(*elem))
	}

	terminator := (**C.char)(unsafe.Pointer(uintptr(unsafe.Pointer(argv)) + uintptr(len(args))*unsafe.Sizeof(*argv)))
	*terminator = (*C.char)(unsafe.Pointer(nil))

	diags := (*C.char)(unsafe.Pointer(nil))
	defer C.free(unsafe.Pointer(diags))

	success := C.clang_compile(argv, C.ulong(len(args)), &diags)
	return CompileResult{
		success: bool(success),
		diags:   C.GoString(diags),
	}
}
