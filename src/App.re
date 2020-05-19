module Window = {
  type t;
  [@bs.val] external window: t = "window";
  [@bs.get] external innerWidth: t => int;
  [@bs.get] external innerHeight: t => int;
  [@bs.set] external onResize: (t, unit => unit) => unit = "onresize";
};

type state = {
  source: string,
  result: Belt.Result.t(string, exn),
  worker: option(Worker.worker),
};

type action =
  | EditorValueChanged(string)
  | WorkerCreated(Worker.worker)
  | WorkerMessageReceived(Bridge.fromWorker)
  | WindowResized;

let initialReasonReact = "module Greeting = {
  [@react.component]
  let make = () => {
    <button> {React.string(\"Hello!\")} </button>
  };
};

ReactDOMRe.renderToElementWithId(<Greeting />, \"preview\");";

let initialState = {
  source: initialReasonReact,
  result: Ok(""),
  worker: None,
};

let reducer = (state, action) =>
  switch (action) {
  | EditorValueChanged(code) => {...state, source: code}
  | WorkerCreated(worker) => {...state, worker: Some(worker)}
  | WorkerMessageReceived({result}) => {...state, result}
  | WindowResized => {...state, source: state.source} /* Return new copy so component re-renders (without having to add window size to state) */
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

  React.useEffect0(() => {
    Window.(window->onResize(() => {dispatch @@ WindowResized}));
    let worker = Bridge.make();
    dispatch @@ WorkerCreated(worker);
    worker->Bridge.App.onMessage(res =>
      dispatch @@ WorkerMessageReceived(res##data)
    );
    // worker->Toplevel.Top.onErrorFromWorker(Js.log);
    None;
  });

  React.useEffect2(
    () => {
      switch (state.worker) {
      | Some(worker) => worker->Bridge.App.postMessage({code: state.source})
      | None => ()
      };
      None;
    },
    (state.source, state.worker),
  );

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
        switch (state.result) {
        | Ok(res) => res
        | Error(_err) => "err"
        }
      }
      readOnly=true
    />
  </main>;
};
