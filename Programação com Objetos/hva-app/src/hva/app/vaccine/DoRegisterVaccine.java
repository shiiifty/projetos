package hva.app.vaccine;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.DuplicateVaccineKeyException;
import hva.app.exceptions.UnknownSpeciesKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
//FIXME import other classes if needed

class DoRegisterVaccine extends Command<Hotel> {

    DoRegisterVaccine(Hotel receiver) {
        super(Label.REGISTER_VACCINE, receiver);
    }

    @Override
    protected final void execute() throws CommandException {
        Form request = new Form();
       try {
        request.addStringField("vaccineId", Prompt.vaccineKey());
        request.addStringField("vaccineName", Prompt.vaccineName());
        request.addStringField("vaccineApplicable", Prompt.listOfSpeciesKeys());
        request.parse();
        //if (_receiver.verifySpecies(request.stringField("vaccineApplicable"))) {
             //_receiver.giveSpecie(request.stringField("vaccineApplicable"));
        //}
        _receiver.registerVaccine(request.stringField("vaccineId"), request.stringField("vaccineName"), request.stringField("vaccineApplicable"));
       } catch (hva.exceptions.DuplicateVaccineKeyException e) {throw new DuplicateVaccineKeyException(request.stringField("vaccineId"));}
       catch (hva.exceptions.UnknownSpeciesKeyException e) {throw new UnknownSpeciesKeyException( _receiver.giveSpecie(request.stringField("vaccineApplicable")));}
    }

}