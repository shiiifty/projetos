package hva.exceptions;

import java.io.Serial;


public class UnknownSpeciesKeyException extends Exception {
    @Serial
    private static final long serialVersionUID = 202407081733L;

    private final String key;

    public UnknownSpeciesKeyException(String key) {
        this.key = key;
    }

    public String getKey() {
        return key;
    }
}