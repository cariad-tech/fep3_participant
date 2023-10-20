# Second stage is needed to avoid having credentials in production image
FROM ubuntu:18.04
# Install packages for network latency testing
RUN apt-get update && \
    apt-get install sudo iproute2 iputils-ping perl elfutils vtable-dumper ctags -y
# Create a user group
RUN groupadd fep
# Create user "fep" with password "fep"
RUN useradd -rm -d /home/fep -s /bin/bash -g fep -G sudo -p gUQm6Qta8XCJg -u 1001 fep
WORKDIR /home/fep
# Set final user

ARG SCRIPTPATH=default
# HEADERSFILE should be always a filename, not a path
ARG HEADERSFILE=default
ARG DUMP_COMMAND=default
ARG PART_LIB_PATH=default
ARG OUT_DUMP_PATH=default
ARG COMPL_CHECK_COMMAND=default
ARG REF_ABI_DUMP=default
ARG REPORT_PATH=default
ARG PLUGIN_LIB_PATH=default
ARG PLUGIN_OUT_DUMP_PATH=default
ARG PLUGIN_REF_ABI_DUMP=default

#ENV SCRIPT_ARGS=$DUMP_COMMAND $PART_LIB_PATH /opt/$HEADERSFILE $OUT_DUMP_PATH



ENV SCRIPT_ARGS ''$DUMP_COMMAND' '$PART_LIB_PATH' /opt/'$HEADERSFILE' '$OUT_DUMP_PATH' '$COMPL_CHECK_COMMAND' '$REF_ABI_DUMP' '$REPORT_PATH' '$PLUGIN_LIB_PATH' '$PLUGIN_OUT_DUMP_PATH' '$PLUGIN_REF_ABI_DUMP''
RUN echo "CHECK VAR $SCRIPT_ARGS"

USER fep
# Copy bash script
COPY ${SCRIPTPATH} /opt
COPY ${HEADERSFILE} /opt
#b=$(basename $a)

USER root
RUN  /bin/bash -c 'chmod +x /opt/abi_check.sh'
USER fep

# set perl environment variables
ENV PERL_PATH=/my-software/perl/
ENV PERL5LIB=$PERL_PATH:$PERL_PATH/lib/perl5:$PERL5LIB
ENV PERL_MM_OPT="INSTALL_BASE=$PERL_PATH"
ENV PERL_MB_OPT="--install_base $PERL_PATH"
ENV PATH="$PERL_PATH/bin:$PATH"

CMD ["sh", "-c","/opt/abi_check.sh $SCRIPT_ARGS"]
