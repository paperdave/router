import { NativeRouter } from "./src";

// memoirist-like router written in c + ffi
const x = new NativeRouter<string>();
x.add("GET", "/api/ffi", "first");
x.add("GET", "/api/abc", "second");
console.log(x.toJSON());
