import { Router } from "./src";

const x = new Router<string>();
x.add("GET", "/api/ffi", "first");
x.add("GET", "/api/abc", "second");