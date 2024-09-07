# PPX explorer 

See how the resulting code after applying the most used PPXs transformations: 

https://ml-in-barcelona.github.io/ppx-explorer/

(note: Currently targeting Reason syntax and ReasonReact PPX exclusively, PRs welcomed!)

## Run Project

```sh
yarn install
yarn start
# in another tab
yarn server
```

View the app in the browser at http://localhost:8000. Running in this environment provides hot reloading and support for routing; just edit and save the file and the browser will automatically refresh.

To use a port other than 8000 set the `PORT` environment variable (`PORT=8080 yarn server`).

## Build for Production

```sh
yarn clean
yarn build
yarn webpack:production
```

This will replace the development artifact `docs/Index.js` for an optimized version as well as copy `src/index.html` into `docs/`. You can then deploy the contents of the `docs` directory (`index.html` and `Index.js`).
