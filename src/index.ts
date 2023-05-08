import { Pointer, toArrayBuffer } from "bun:ffi";
import { ffi } from "./binding";

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

export interface JSONNode<T> {
  store: T;
  part: string;
  children: Record<string, JSONNode<T>>;
  param?: JSONNode<T>;
  wildcard?: JSONNode<T>;
}
export type JSONRouter<T> = Record<string, JSONNode<T>>;

export class NativeRouter<T> {
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

  toJSON(): JSONRouter<T> {
    let routes = this.#routes;
    const obj = JSON.parse("" + ffi.router_print_json(this.#ptr));
    function traverse(obj: any) {
      if (obj.id) {
        obj.store = routes[obj.id - 1]?.store;
      }
      delete obj.id;
      for (const key in obj.children) {
        traverse(obj.children[key]);
      }
      if (obj.param) traverse(obj.param);
      if (obj.wildcard) traverse(obj.wildcard);
    }
    for (const key in obj) {
      traverse(obj[key]);
    }
    return obj as JSONRouter<T>;
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
