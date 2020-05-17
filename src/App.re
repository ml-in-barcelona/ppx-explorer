module Window = {
  type t;
  [@bs.val] external window: t;
  [@bs.get] external innerWidth: t => int;
  [@bs.get] external innerHeight: t => int;
  [@bs.set] external onResize: (t, unit => unit) => unit = "onresize";
};

type state = {source: string};

type action =
  | EditorValueChanged(string)
  | WindowResized;

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

let fromSource = code =>
  try({
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
    Ok(Format.flush_str_formatter());
  }) {
  | exn =>
    Js.log(exn);
    Error(exn);
  };

let initialReasonReact = "module Greeting = {
  [@react.component]
  let make = () => {
    <button> {React.string(\"Hello!\")} </button>
  };
};

ReactDOMRe.renderToElementWithId(<Greeting />, \"preview\");";

let initialState = {source: initialReasonReact};

let reducer = (state, action) =>
  switch (action) {
  | EditorValueChanged(code) => {source: code}
  | WindowResized => {source: state.source} /* Return new copy so component re-renders (without having to add window size to state) */
  };

module Style = {
  let globalPadding = 20;
  let titleHeight = 150;
  let innerPadding = 20;
};
module Editor = {
  [@react.component]
  let make = (~left, ~width, ~height, ~value, ~readOnly=false, ~onChange=?) => {
    let width = width->string_of_int ++ "px";
    let height = height->string_of_int ++ "px";
    let style =
      ReactDOMRe.Style.make(
        ~position="absolute",
        ~top=Style.(globalPadding + titleHeight)->string_of_int ++ "px",
        ~left=left->string_of_int ++ "px",
        ~width,
        ~height,
        (),
      );
    let options = readOnly ? Some({"readOnly": true}) : None;
    let onChange =
      onChange->Belt.Option.map((onChange, code, _) => {onChange(code)});

    <section style>
      <ReactMonaco.Editor width height value ?onChange ?options />
    </section>;
  };
};

[@react.component]
let make = () => {
  let (state, dispatch) = React.useReducer(reducer, initialState);

  React.useEffect(() => {
    Window.(window->onResize(() => {dispatch @@ WindowResized}));
    None;
  });

  let processed = fromSource(state.source);

  let width = Window.(window->innerWidth) - 2 * Style.globalPadding;
  let height = Window.(window->innerHeight);
  let halfWidth =
    Js.Math.floor(((width - Style.innerPadding) / 2)->float_of_int);
  let remainingHeight = Style.(height - titleHeight - globalPadding);

  <main>
    <h1> {React.string("PPX explorer")} </h1>
    <p>
      {React.string(
         "See the transformation from the most used PPXs with live code.",
       )}
    </p>
    <strong> {React.string("ReasonReact PPX")} </strong>
    <Editor
      left=Style.globalPadding
      width=halfWidth
      height=remainingHeight
      value={state.source}
      onChange={code => dispatch @@ EditorValueChanged(code)}
    />
    <Editor
      left=Style.(globalPadding + innerPadding + halfWidth)
      width=halfWidth
      height=remainingHeight
      value={
        switch (processed) {
        | Ok(res) => res
        | Error(_err) => "err"
        }
      }
      readOnly=true
    />
  </main>;
};
