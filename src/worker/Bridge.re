module Types = {
  type fromApp = {code: string};
  type fromWorker = {result: Belt.Result.t(string, exn)};
  [@bs.new] [@bs.module]
  external make: unit => Worker.worker = "worker-loader!./RefmtWorker.bs.js";
};

include Worker.Make(Types);
