package hva.app.employee;

import pt.tecnico.uilib.forms.Form;

import hva.Hotel;
import hva.app.exceptions.DuplicateEmployeeKeyException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;


class DoRegisterEmployee extends Command<Hotel> {

    DoRegisterEmployee(Hotel receiver) {
        super(Label.REGISTER_EMPLOYEE, receiver);
    
    }

    @Override
    protected void execute() throws CommandException {
        Form request = new Form();
        try {
            request.addStringField("id", Prompt.employeeKey());
            request.addStringField("name", Prompt.employeeName());
            request.addOptionField("type", Prompt.employeeType(), "VET", "TRT");
            request.parse();
            _receiver.registerEmployee(request.stringField("type"), request.stringField("id"), request.stringField("name"), "");
        } catch (hva.exceptions.DuplicateEmployeeKeyException e) {throw new DuplicateEmployeeKeyException(request.stringField("id"));} 
    }

}