package hva.app.habitat;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.UnknownHabitatKeyException;
import hva.app.exceptions.UnknownSpeciesKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoChangeHabitatInfluence extends Command<Hotel> {

    DoChangeHabitatInfluence(Hotel receiver) {
        super(Label.CHANGE_HABITAT_INFLUENCE, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("habitatId", Prompt.habitatKey());
            request.addStringField("specieId", hva.app.animal.Prompt.speciesKey());
            request.addOptionField("influence", Prompt.habitatInfluence(), "POS", "NEU", "NEG");
            request.parse();
            _receiver.changeHabitatInfluence(request.stringField("habitatId"), request.stringField("specieId"), request.stringField("influence"));
        } catch (hva.exceptions.UnknownHabitatKeyException e) {throw new UnknownHabitatKeyException(request.stringField("habitatId"));}
        catch (hva.exceptions.UnknownSpeciesKeyException e) {throw new UnknownSpeciesKeyException(request.stringField("specieId"));}
    }

}