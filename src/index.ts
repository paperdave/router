import { Pointer, toArrayBuffer } from "bun:ffi";
import { ffi } from "./lib";

const td = new TextDecoder();
const te = new TextEncoder();
const decodeText = (x: BufferSource) => td.decode(x);
const encodeText = (x: string) => te.encode(x);

const bufferPtr = ffi.parambuf_init();
if (bufferPtr === 0) {
  throw new Error("Failed to initialize");
}
const returnBuffer = toArrayBuffer(bufferPtr, 0, 2048);
const dataView = new DataView(returnBuffer);

export interface FindResult<
  T,
  Params extends { [key: string]: string } = { [key: string]: string }
> {
  store: T;
  params: Params;
}

export class Router<T> {
  #ptr: Pointer;
  #routes: { store: T; paramNames: string[] }[] = [];

  constructor() {
    this.#ptr = ffi.router_new();
    if (this.#ptr === 0) {
      throw new Error("Failed to create router."); // TODO: error passing
    }
  }

  add(method: string, route: string, store: T): T {
    if (method.includes("/")) throw new Error("Invalid method name");
    if (!route.startsWith("/")) throw new Error("Route must start with '/'");

    var routes = this.#routes;
    var result = ffi.router_add(
      this.#ptr,
      encodeText(method + route) as any,
      routes.length + 1
    );
    if (result === -1) {
      throw new Error("" + ffi.jserror_get());
    }
    var number = dataView.getInt8(0);
    var paramNames = [];
    var length;
    var offset = 1;
    while (number--) {
      length = dataView.getInt8(offset);
      offset++;
      paramNames.push(decodeText(returnBuffer.slice(offset, offset + length)));
      offset += length;
    }
    routes.push({ store, paramNames });
    return store;
  }

  find(method: string, pathname: string): FindResult<T> | null {
    var result = ffi.router_find(
      this.#ptr,
      encodeText(method + pathname) as any
    );
    if (result === 0) {
      return null;
    }
    if (!result) {
      throw new Error("" + ffi.jserror_get());
    }
    var { store, paramNames } = this.#routes[result - 1];
    var params: any = {};
    var length = dataView.getInt8(0);
    var offset = 1;
    for (var i = 0; i < length; i++) {
      params[paramNames[i]] = decodeText(
        returnBuffer.slice(offset, offset + dataView.getInt8(offset))
      );
      offset += dataView.getInt8(offset) + 1;
    }
    return { store, params };
  }

  print() {
    ffi.router_print(this.#ptr);
  }

  free() {
    ffi.router_free(this.#ptr);
    this.#ptr = 0;
    this.#routes = [];
    this.add = this.find = () => {
      throw new Error("Router is freed");
    };
  }
}
