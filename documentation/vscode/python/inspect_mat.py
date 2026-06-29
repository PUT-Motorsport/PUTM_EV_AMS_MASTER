import scipy.io
import numpy as np
from scipy.io.matlab import mat_struct


def inspect(obj, name="root", indent=0):
    prefix = " " * indent

    # MATLAB struct
    if isinstance(obj, mat_struct):
        print(f"{prefix}{name}: struct")
        for field in obj._fieldnames:
            inspect(getattr(obj, field), field, indent + 4)

    # Dictionary
    elif isinstance(obj, dict):
        print(f"{prefix}{name}: dict")
        for key, value in obj.items():
            if not key.startswith("__"):
                inspect(value, key, indent + 4)

    # NumPy array
    elif isinstance(obj, np.ndarray):
        print(f"{prefix}{name}: ndarray shape={obj.shape}, dtype={obj.dtype}")

        # MATLAB cell arrays or struct arrays
        if obj.dtype == object:
            for idx, item in np.ndenumerate(obj):
                inspect(item, f"{name}{idx}", indent + 4)

    # Primitive types
    elif isinstance(obj, (int, float, complex, str, bool, np.number)):
        print(f"{prefix}{name}: {type(obj).__name__}")

    else:
        print(f"{prefix}{name}: {type(obj).__name__}")



if __name__ == "__main__":

    data = scipy.io.loadmat(
    "Molicel_INR21700P50B_241c01111_data.mat",
    struct_as_record=False,
    squeeze_me=True
    )

    inspect(data)