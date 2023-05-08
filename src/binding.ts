// generated with codegen.ts, run `make` to rebuild
import { FFIType, dlopen, suffix } from "bun:ffi";
import path from "path";

const library = dlopen(path.join(import.meta.dir, `router.${suffix}`), {
  router_add: {
    args: [FFIType.pointer, FFIType.cstring, FFIType.u32],
    returns: FFIType.i32,
  },
  router_find: {
    args: [FFIType.pointer, FFIType.cstring],
    returns: FFIType.u32,
  },
  jserror_get: {
    returns: FFIType.cstring,
  },
  parambuf_init: {
    returns: FFIType.pointer,
  },
  parambuf_free: {

  },
  router_print: {
    args: [FFIType.pointer],
  },
  router_print_json: {
    args: [FFIType.pointer],
    returns: FFIType.cstring,
  },
  router_new: {
    returns: FFIType.pointer,
  },
  router_free: {
    args: [FFIType.pointer],
  },
});
if (!library.symbols) {
  throw library;
}

export const ffi = library.symbols;
