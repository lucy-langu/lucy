# Lucy Editor Syntax Support

This directory contains lightweight syntax definitions for popular editors.

## VS Code

Copy `editors/vscode` into a VS Code extension directory, or package it as an extension:

```bash
cd editors/vscode
npx @vscode/vsce package
```

The extension registers `.lucy` and provides keyword, string (single and double quote), comment, number, operator, interpolation and function highlighting.

## Vim / Neovim

Add the files under `editors/vim` to your Vim runtime, for example:

```text
~/.vim/after/syntax/lucy.vim
~/.vim/ftdetect/lucy.vim
```

## Sublime Text

Copy `editors/sublime/Lucy.sublime-syntax` into the user's `Packages/User` directory.

## Helix

Merge the language definition from `editors/helix/languages.toml` into `~/.config/helix/languages.toml`.

## Emacs

Load `editors/emacs/lucy-mode.el`; it registers `.lucy` automatically and provides keyword, string, comment, interpolation, number and function highlighting.
