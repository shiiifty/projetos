package hva.app.habitat;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.DuplicateTreeKeyException;
import hva.app.exceptions.UnknownHabitatKeyException;
import hva.trees.TreeType;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoAddTreeToHabitat extends Command<Hotel> {

    DoAddTreeToHabitat(Hotel receiver) {
        super(Label.ADD_TREE_TO_HABITAT, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try { 
            request.addStringField("habitatId", Prompt.habitatKey());
            request.addStringField("id", Prompt.treeKey());
            request.addStringField("name", Prompt.treeName());
            request.addIntegerField("age", Prompt.treeAge());
            request.addIntegerField("difficulty", Prompt.treeDifficulty());
            request.addOptionField("type", Prompt.treeType(), "CADUCA", "PERENE");
            request.parse();
            _display.popup(_receiver.addTreeToHabitat(request.stringField("habitatId"), request.stringField("id"), request.stringField("name"), request.integerField("age"), request.integerField("difficulty"), request.stringField("type")));
        } catch (hva.exceptions.DuplicateTreeKeyException e) {throw new DuplicateTreeKeyException(request.stringField("id"));}
        catch (hva.exceptions.UnknownHabitatKeyException e) {throw new UnknownHabitatKeyException(request.stringField("habitatId"));}
    }
}