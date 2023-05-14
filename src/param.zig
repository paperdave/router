const std = @import("std");
const testing = std.testing;

pub const Param = struct {
    pub const Kind = enum {
        static,
        param,
        wildcard,
        wildcard_optional,
    };
    kind: Kind,
    name: []const u8,
};

inline fn validParamChar(char: u8) bool {
    return char < 'a' or char > 'z' and
        char < 'A' or char > 'Z' and
        char < '0' or char > '9' and
        char != '-' and
        char != '_';
}

const ParseError = error{ InvalidPathCharacter, InvalidParam, NotImplemented };

const Parser = struct {
    allocator: std.mem.Allocator,
    path: []const u8,
    i: usize,
    buffer: []u8,

    pub fn next(this: *Parser) ParseError!?Param {
        if (this.i >= this.path.len) {
            return null;
        }
        var i = this.i;
        defer this.i = i;
        switch (this.path[this.i]) {
            ':' => {
                i += 1;
                if (i >= this.path.len) {
                    return ParseError.InvalidParam;
                }
                while (i < this.path.len) : (i += 1) {
                    var char = this.path[i];
                    if (char == '/') {
                        break;
                    }
                    if (!validParamChar(char)) {
                        return ParseError.InvalidPathCharacter;
                    }
                }
                return Param{
                    .kind = .param,
                    .name = this.path[this.i + 1 .. i],
                };
            },
            '[' => {
                const isDouble = brk: {
                    if (this.path[i + 1] == '[') {
                        i += 1;
                        break :brk true;
                    } else {
                        break :brk false;
                    }
                };
                const isWildcard = brk: {
                    if (this.path[i + 1] == '.' and this.path[i + 2] == '.' and this.path[i + 3] == '.') {
                        i += 3;
                        break :brk true;
                    } else {
                        break :brk false;
                    }
                };

                if (isDouble and !isWildcard) {
                    return ParseError.InvalidParam;
                }

                const start = i + 1;

                while (i < this.path.len) : (i += 1) {
                    var char = this.path[i];
                    if (char == ']') {
                        const slice = this.path[start..i];
                        if (isDouble) {
                            if (i + 1 >= this.path.len or this.path[i + 1] != ']') {
                                return ParseError.InvalidParam;
                            }
                            i += 2;
                        } else {
                            i += 1;
                        }

                        if (isWildcard and i != this.path.len) {
                            return ParseError.InvalidParam;
                        }

                        return Param{
                            .kind = if (isWildcard)
                                if (isDouble)
                                    Param.Kind.wildcard_optional
                                else
                                    Param.Kind.wildcard
                            else
                                Param.Kind.param,
                            .name = slice,
                        };
                    }
                    // must use from a-z, A-Z, 0-9, -, _
                    if (!validParamChar(char)) {
                        return ParseError.InvalidPathCharacter;
                    }
                }
                return error.InvalidParam;
            },
            '*' => {
                if (i + 1 != this.path.len) {
                    return ParseError.InvalidParam;
                }
                return Param{
                    .kind = .wildcard,
                    .name = this.path[i + 1 .. i + 1],
                };
            },
            else => {
                var backslashes: usize = 0;
                while (i < this.path.len) : (i += 1) {
                    switch (this.path[i]) {
                        '\\' => {
                            i += 1;
                            if (i >= this.path.len) {
                                return ParseError.InvalidPathCharacter;
                            }
                            backslashes += 1;
                            this.buffer[i - backslashes] = this.path[i];
                        },
                        ':', '[', '*' => {
                            break;
                        },
                        else => {
                            this.buffer[i - backslashes] = this.path[i];
                        },
                    }
                }
                return Param{
                    .kind = .static,
                    .name = this.buffer[this.i .. i - backslashes],
                };
            },
        }
    }

    pub fn deinit(this: Parser) void {
        this.allocator.free(this.buffer);
    }
};

pub fn parse(path: []const u8, allocator: std.mem.Allocator) !Parser {
    return Parser{
        .allocator = allocator,
        .buffer = try allocator.alloc(u8, path.len),
        .path = path,
        .i = 0,
    };
}

const test_allocator = std.heap.page_allocator;

fn expectParam(
    optional: ?Param,
    kind: Param.Kind,
    name: []const u8,
) !void {
    if (optional) |param| {
        testing.expect(param.kind == kind) catch |err| {
            std.log.err("expected param {s}, got {s}", .{ @tagName(kind), @tagName(param.kind) });
            return err;
        };
        testing.expect(std.mem.eql(u8, param.name, name)) catch |err| {
            std.log.err("expected param name \"{s}\", got \"{s}\"", .{ name, param.name });
            return err;
        };
    } else {
        std.log.err("expected param {s} with \"{s}\", got none", .{
            @tagName(kind),
            name,
        });
        return error.TestUnexpectedResult;
    }
}

fn printParam(optional: ?Param) void {
    if (optional) |param| {
        std.log.info("param: {s} \"{s}\"", .{ @tagName(param.kind), param.name });
    } else {
        std.log.err("expected param, got none", .{});
    }
}

test "parse static" {
    var path = try parse("/foo/bar", test_allocator);
    try expectParam(try path.next(), Param.Kind.static, "/foo/bar");
}

test "parse static escaped" {
    var path = try parse("/foo/\\:foo", test_allocator);
    try expectParam(try path.next(), Param.Kind.static, "/foo/:foo");
}

test "parse colon param" {
    var path = try parse("/foo/:param", test_allocator);
    try expectParam(try path.next(), Param.Kind.static, "/foo/");
    try expectParam(try path.next(), Param.Kind.param, "param");
}

test "parse colon param 2" {
    var path = try parse("/foo/:param/okay", test_allocator);
    try expectParam(try path.next(), Param.Kind.static, "/foo/");
    try expectParam(try path.next(), Param.Kind.param, "param");
    try expectParam(try path.next(), Param.Kind.static, "/okay");
}

test "parse bracket param" {
    var path = try parse("/foo/[etc].json", test_allocator);
    try expectParam(try path.next(), Param.Kind.static, "/foo/");
    try expectParam(try path.next(), Param.Kind.param, "etc");
    try expectParam(try path.next(), Param.Kind.static, ".json");
}

test "parse bracket param 2" {
    var path = try parse("/foo/[etc]", test_allocator);
    try expectParam(try path.next(), Param.Kind.static, "/foo/");
    try expectParam(try path.next(), Param.Kind.param, "etc");
}

test "parse bracket wildcard" {
    var path = try parse("/foo/[...etc]", test_allocator);
    try expectParam(try path.next(), Param.Kind.static, "/foo/");
    try expectParam(try path.next(), Param.Kind.wildcard, "etc");
}

test "parse bracket optional wildcard" {
    var path = try parse("/foo/[[...etc]]", test_allocator);
    try expectParam(try path.next(), Param.Kind.static, "/foo/");
    try expectParam(try path.next(), Param.Kind.wildcard_optional, "etc");
}

test "parse bracket optional wildcard" {
    var path = try parse("/foo/*", test_allocator);
    try expectParam(try path.next(), Param.Kind.static, "/foo/");
    try expectParam(try path.next(), Param.Kind.wildcard_optional, "etc");
}

pub fn main() !void {
    var path = try parse("/foo/[[...param]]", test_allocator);
    printParam(try path.next());
    printParam(try path.next());
}
