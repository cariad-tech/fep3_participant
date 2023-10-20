import os
from pathlib import Path

class ResultWriter:
    def __init__(self, build_dir, abi_dump = True, abi_check = False):
        if abi_check:
            self.name = "abi_check"
        elif abi_dump:
            self.name = "abi_dump"
        else:
            raise Exception("Either abi_dump or abi_check should be defined")

        result_file_name = "test_" + self.name  +  ".xml"
        self.result_file_path = os.path.join(build_dir, "test", "result", result_file_name)

    def write_ok(self):
        result_ok = "<testsuite name=\"%s\">\n <testcase classname=\"%s\" name=\"%s\"/>\n</testsuite>" % \
        (self.name, self.name, self.name)
        self.__write_to_file(result_ok)

    def write_error(self, message):
        self.__write_error_impl(message)

    def write_exception_error(self, exception_message):
        system_out = "        <system-out>\n"
        system_out += exception_message
        system_out += "\n        </system-out>\n"
        self.__write_error_impl("exception", system_out)

    def __write_error_impl(self, message, system_out = ""):
        result_error = "<testsuite name=\"%s\">\n    <testcase classname=\"%s\" name=\"%s\">\n" \
            "        <error message=\"%s\" type=\"Error\"></error>\n" % \
            (self.name,  self.name, self.name, message)
        result_error += system_out
        result_error += "    </testcase>\n</testsuite>"
        self.__write_to_file(result_error)

    def __write_to_file(self, file_content):
        directory = os.path.dirname(self.result_file_path)
        if not os.path.exists(directory):
            os.makedirs(directory)

        f = open(self.result_file_path, "w")
        f.write(file_content)
        f.close()
