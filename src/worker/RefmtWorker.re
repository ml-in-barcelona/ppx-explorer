module Converter =
  Reason_toolchain_packed.Migrate_parsetree.Convert(
    Reason_toolchain_packed.Migrate_parsetree.OCaml_408,
    Reason_toolchain_packed.Migrate_parsetree.OCaml_406,
  );
module ConverterBack =
  Reason_toolchain_packed.Migrate_parsetree.Convert(
    Reason_toolchain_packed.Migrate_parsetree.OCaml_406,
    Reason_toolchain_packed.Migrate_parsetree.OCaml_408,
  );

Bridge.Worker.(
  self->onMessage(data =>
    try({
      let {Bridge.Types.code} = data##data;
      let lexbuf = Lexing.from_string(code);
      let (_ast, comments) =
        lexbuf |> Refmt_api.RE.implementation_with_comments;
      let reactAst =
        lexbuf
        ->Reason_toolchain_packed.Reason_toolchain.RE.implementation
        ->Converter.copy_structure;
      let newAst = ReasonReactPpx.rewrite_implementation(reactAst);
      Refmt_api.RE.print_implementation_with_comments(
        Format.str_formatter,
        (newAst->ConverterBack.copy_structure, comments),
      );
      postMessage({result: Ok(Format.flush_str_formatter())});
    }) {
    | exn =>
      Js.log(exn);
      postMessage({result: Error(exn)});
    }
  )
);
