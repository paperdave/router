const std = @import("std");
const testing = std.testing;

const Store = i32;

const charToRadix = [128]i8{
    // first 32 chars are not used
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0x00-0x0f
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0x10-0x1f
    // # is not url safe, * is internal
    0, 1, 2, -1, 3, 4, 5, 6, 7, 8, 100, 9, 10, 11, 12, 13, // 0x20-0x2f
    // ? is not url safe, and : is internal
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, -1, 24, 25, 26, 27, 101, // 0x30-0x3f
    28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, // 0x40-0x4f
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, // 0x50-0x5f
    // alphabet comes around again, but lowercase
    60, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, // 0x60-0x6f
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, -1, -1, -1, -1, -1, // 0x70-0x7f
};
const radixToChar = brk: {
    var result: [256]u8 = .{};
    var i = 0;
    while (i < 128) : (i += 1) {
        var radix = charToRadix[i];
        if (radix != -1) {
            result[radix] = i;
        } else {}
    }
    break :brk result;
};
const radixLBracket = charToRadix['['];
const radixRBracket = charToRadix[']'];
const radixColon = charToRadix[':'];
const radixStar = charToRadix['*'];

/// An altered radix tree that allows for parametric and wildcard routes.
const Router = struct {
    const MatchedRoute = struct {
        store: Store,
        // params: [][]const u8,
    };

    const Node = struct {
        part: []const i8,
        store: ?Store,
        children: [61]?*Node,
        param: ?*Node,
        wildcard: ?*Node,
    };

    children: [61]?*Node,
    arena: std.heap.ArenaAllocator,
    alloc: std.mem.Allocator,

    pub fn init(allocator: std.mem.Allocator) Router {
        var arena = std.heap.ArenaAllocator.init(allocator);
        return Router{
            .children = .{},
            .arena = arena,
            .alloc = arena.allocator(),
        };
    }

    pub fn deinit(self: *const Router) void {
        self.arena.deinit();
    }

    pub fn add(self: *const Router, path: []const u8, store: Store) !void {
        var mappedPath: [path.len]i8 = undefined;
        if (path.len == 0) {
            return error.InvalidRoute;
        }
        for (path, 0..) |value, i| {
            if (value > 127) {
                return error.InvalidRoute;
            }
            mappedPath[i] = charToRadix[value];
            if (mappedPath[i] == -1) {
                return error.InvalidRoute;
            }
        }
        if (self.children[mappedPath[0]] == null) {
            self.children[mappedPath[0]] = self.allocator.create(Node){
                .part = mappedPath[1..],
                .store = store,
                .children = .{},
                .param = null,
                .wildcard = null,
            };
        } else {}
    }

    pub fn find(self: *const Router, path: []const u8) ?MatchedRoute {
        _ = path;
        _ = self;
        return null;
    }
};

test {
    const allocator = std.heap.page_allocator;
    var router = Router.init(allocator);
    defer router.deinit();
}
