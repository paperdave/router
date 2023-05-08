// generated with codegen.ts, run `make` to rebuild
import { FFIType, dlopen, suffix } from "bun:ffi";
import path from "path";

const library = dlopen(path.join(import.meta.dir, `router.${suffix}`), {
  
});
if (!library.symbols) {
  throw library;
}

export const ffi = library.symbols;
