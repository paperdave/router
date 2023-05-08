# bun ffi router

HTTP router library for Bun written in native C and FFI

Implementation roughly on memoirist by SaltyAOM

Unfortunatly, even when writing in C, it performs much slower than memoirist due to the overhead of FFI calls and encoding a buffer, so this project remains unfinished. Might finish it just to say I can write a Radix Tree, but meh.

```ts
import { NativeRouter } from "./src";

const x = new NativeRouter<{ id: string }>();
x.add("GET", "/users/:id", { id: "get data" });
x.add("GET", "/users/:id/name", { id: "get name" });

console.log(x.find("GET", "/users/dave/name"));
// -> { store: { id: "get name" }, params: { user: "dave" } }
```

This is intended to be used by framework authors, as it only does routing, and not have a server, handlers, requests, etc. It instead just maps a method and path to an arbitrary value. You can put route metadata or a callback function there.
