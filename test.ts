import { bench, group, run } from "mitata";
import { Memoirist } from "memoirist";
import { NativeRouter } from "./src";

group("simple (no params)", () => {
  const native = new NativeRouter<string>();
  native.add("GET", "/api/ffi", "first");
  native.add("GET", "/api/abc", "second");
  bench("native", () => {
    native.find("GET", "/api/ffi");
  });
  console.log(native.toJSON());
  const memoirist = new Memoirist();
  memoirist.add("GET", "/api/ffi", "first");
  memoirist.add("GET", "/api/abc", "second");
  bench("memoirist", () => {
    memoirist.find("GET", "/api/ffi");
  });
});
run();
