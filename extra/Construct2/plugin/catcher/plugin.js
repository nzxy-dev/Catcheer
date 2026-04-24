
assert2(cr, "cr namespace not found");

cr.plugins_.catcher = function(runtime) {
    this.runtime = runtime;
};