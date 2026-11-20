const vscode = require('vscode');

function activate(context) {
  let disposable = vscode.commands.registerCommand('lucy.runFile', function () {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
      vscode.window.showInformationMessage('error');
      return;
    }
    const filePath = editor.document.uri.fsPath;
    const terminal = vscode.window.createTerminal('Lucy Runner');
    terminal.show(true);
    terminal.sendText(`lucy "${filePath}"`);
  });

  context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = { activate, deactivate };