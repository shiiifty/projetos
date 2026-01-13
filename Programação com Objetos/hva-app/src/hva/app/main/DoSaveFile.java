package hva.app.main;

import hva.HotelManager;
import hva.exceptions.ImportFileException;
import hva.exceptions.MissingFileAssociationException;
import hva.exceptions.UnavailableFileException;

import java.io.FileNotFoundException;
import java.io.IOException;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

import java.io.IOException;
//FIXME import other classes if needed

class DoSaveFile extends Command<HotelManager> {
    DoSaveFile(HotelManager receiver) {
        super(Label.SAVE_FILE, receiver, r -> r.getHotel() != null);
    }

    @Override
    protected final void execute() throws CommandException {
    	try {
            _receiver.save();
          } catch (MissingFileAssociationException e) {
            try {
              _receiver.saveAs(Form.requestString(Prompt.newSaveAs()));
            } catch (MissingFileAssociationException e1) {
              // TODO Auto-generated catch block
              e1.printStackTrace();
            } catch (IOException e1) {
                e1.printStackTrace();
            }

          } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
          }
    }
}