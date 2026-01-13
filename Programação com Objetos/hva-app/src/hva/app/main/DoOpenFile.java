package hva.app.main;

import hva.HotelManager;
import hva.app.exceptions.FileOpenFailedException;
import hva.exceptions.UnavailableFileException;
import java.io.IOException;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoOpenFile extends Command<HotelManager> {
    DoOpenFile(HotelManager receiver) {
        super(Label.OPEN_FILE, receiver);
    }

    @Override
    protected final void execute() throws CommandException {
       try {
        if (_receiver.changed() && Form.confirm(Prompt.saveBeforeExit())) {
            DoSaveFile cmd = new DoSaveFile(_receiver);
            cmd.execute();
          }
          _receiver.load(Form.requestString(Prompt.openFile()));
        } catch (UnavailableFileException e) {
           _display.popup(Message.fileNotFound());
        } 
    }
}