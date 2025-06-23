import logging

def safe_convert(val, to_type: type):
    """Safely convert a value to a specified type.

    This utility function attempts to convert a value to the specified type.
    If the conversion fails, it logs a critical error message and returns None
    without crashing the program.

    Args:
        val (any): The value to be converted
        to_type (type): The target type for conversion (e.g., int, str, float)

    Returns:
        The converted value if successful, None if conversion fails
    """
    try:
        return to_type(val)
    except:
        print(val)
        logging.critical("Safe convert failed")
