using Microsoft.VisualStudio.Debugger;
using Microsoft.VisualStudio.Debugger.CallStack;
using Microsoft.VisualStudio.Debugger.ComponentInterfaces;
using Microsoft.VisualStudio.Debugger.DefaultPort;
using Microsoft.VisualStudio.Debugger.Evaluation;
using Microsoft.VisualStudio.Debugger.Interop;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;

namespace LeoEngine.VSIX.Folly.Concurrency
{
    public class EvaluateSizeDataItem : DkmDataItem
    {
        public int Size { get; set; }
        public string ErrorMessage { get; set; }
    }

    public class HashMapDataItem : DkmDataItem
    {
        public ulong[] Segments { get; set; }

        public string[] ChildrenPath { get; set; }

        public ulong Size { get; set; }
    }

    class HashmapVisualizerComponent : IDkmCustomVisualizer
    {
        public static readonly Guid GUID = new Guid("44C2656E-9C3C-4CEE-8C6B-D5BD8CD16D0E");

        int EvaluateSize(string target, DkmInspectionContext InspectionContext, DkmStackWalkFrame StackFrame)
        {
            using (var expression = DkmLanguageExpression.Create(InspectionContext.Language, DkmEvaluationFlags.DesignTime, $"sizeof({target})", null))
            {
                var worklist = DkmWorkList.Create(null);
                InspectionContext.EvaluateExpression(worklist, expression, StackFrame, asyncResult =>
                {
                    var item = new EvaluateSizeDataItem();
                    if (asyncResult.ResultObject is DkmSuccessEvaluationResult succResult)
                    {
                        if (int.TryParse(succResult.Value, out var size))
                        {
                            item.Size = size;
                        }
                        else
                        {
                            item.ErrorMessage = $"{succResult.Value} is not a number.";
                        }
                    }
                    if (asyncResult.ResultObject is DkmFailedEvaluationResult failResult)
                    {
                        item.ErrorMessage = $"{ failResult.ErrorMessage}";
                    }
                    asyncResult.ResultObject.InspectionContext.InspectionSession.SetDataItem(DkmDataCreationDisposition.CreateAlways, item);
                });
                worklist.Execute();
            }
            var dataItem = InspectionContext.InspectionSession.GetDataItem<EvaluateSizeDataItem>();
            if (dataItem != null)
            {
                if (!string.IsNullOrEmpty(dataItem.ErrorMessage))
                {
                    Debug.Fail($"InspectionContext.EvaluateExpression Failed: {dataItem.ErrorMessage}");
                }
                return dataItem.Size;
            }
            else
            {
                Debug.Fail($"InspectionContext.EvaluateExpression Failed: Unknown");
            }

            return 0;
        }

        DkmSuccessEvaluationResult Evaluate(DkmVisualizedExpression expr, string text)
        {
            using (DkmLanguageExpression vexpr = DkmLanguageExpression.Create(expr.InspectionContext.Language, DkmEvaluationFlags.None, text, null))
            {
                expr.EvaluateExpressionCallback(expr.InspectionContext, vexpr, expr.StackFrame, out var result);

                if (result is DkmFailedEvaluationResult failResult)
                {
                    Debug.Fail(failResult.ErrorMessage);
                    throw new ArgumentException(text);
                }

                return (DkmSuccessEvaluationResult)result;
            }
        }

        void IDkmCustomVisualizer.EvaluateVisualizedExpression(DkmVisualizedExpression visualizedExpression, out DkmEvaluationResult resultObject)
        {
            var item = new HashMapDataItem();

            var rootExpr = (DkmRootVisualizedExpression)visualizedExpression;

            var pointValue = (DkmPointerValueHome)visualizedExpression.ValueHome;
            var ParentInspectionContext = visualizedExpression.InspectionContext;
            var TargetProcess = visualizedExpression.RuntimeInstance.Process;
            int NumShards = 256;
            //TODO 32bit support
            var segments_ = new byte[NumShards * 8];

            var offsetResult = Evaluate(visualizedExpression, $"(char*)&{rootExpr.FullName}.segments_-(char*)&{rootExpr.FullName}");
            var offset = ulong.Parse(offsetResult.Value);

            TargetProcess.ReadMemory(pointValue.Address + offset, DkmReadMemoryFlags.None, segments_);

            //sizeof(std::mutex) + sizeof(float) + padding + sizeof(size_t)
            // 80                +  4            +  4      + 8
            var segementSize = 104;

            item.Segments = new ulong[NumShards];
            ulong Size = 0;
            for (int i = 0; i != NumShards && segementSize >= 100; ++i)
            {
                var segemntAddress = BitConverter.ToUInt64(segments_, i * 8);
                item.Segments[i] = segemntAddress;

                if (segemntAddress == 0)
                    continue;

                var sgegment = new byte[segementSize];
                TargetProcess.ReadMemory(segemntAddress, DkmReadMemoryFlags.None, sgegment);

                var size = BitConverter.ToUInt64(sgegment, 96);

                Size += size;
            }

            string text = $"{{size = {Size}}}";
            resultObject = DkmSuccessEvaluationResult.Create(
             rootExpr.InspectionContext,
             rootExpr.StackFrame,
             rootExpr.Name,
             rootExpr.FullName,
             DkmEvaluationResultFlags.Expandable | DkmEvaluationResultFlags.ReadOnly,
             text,
             null,
             rootExpr.Type,
             DkmEvaluationResultCategory.Data,
             DkmEvaluationResultAccessType.None,
             DkmEvaluationResultStorageType.None,
             DkmEvaluationResultTypeModifierFlags.None,
             DkmDataAddress.Create(visualizedExpression.RuntimeInstance, pointValue.Address, null),
             null,
             null,
             null);

            item.Size = Size;

            rootExpr.SetDataItem(DkmDataCreationDisposition.CreateAlways, item);
        }

        void IDkmCustomVisualizer.GetChildren(DkmVisualizedExpression visualizedExpression, int initialRequestSize, DkmInspectionContext inspectionContext, out DkmChildVisualizedExpression[] initialChildren, out DkmEvaluationResultEnumContext enumContext)
        {
            var TargetProcess = visualizedExpression.RuntimeInstance.Process;
            var rootExpr = (DkmRootVisualizedExpression)visualizedExpression;

            var item = visualizedExpression.GetDataItem<HashMapDataItem>();

            enumContext = DkmEvaluationResultEnumContext.Create(
                (int)item.Size,
                visualizedExpression.StackFrame,
                visualizedExpression.InspectionContext,
                null);

            initialChildren = new DkmChildVisualizedExpression[0];

            var childrens = new List<string>();
            //collect node address
            for(int i = 0; i!=item.Segments.Length;++i)
            {
                var segemntAddress = item.Segments[i];

                if (segemntAddress == 0)
                    continue;

                var path = $"segments_[{i}].value()->impl_";

                var segementSize = 256; //sizeof(std::mutex) + sizeof(float) + padding + sizeof(size_t) + sizeof(size_t) + padiding64 + sizeof(void*) + sizeof(uint64_t) + sizeof(size_t)
                var sgegment = new byte[segementSize];
                TargetProcess.ReadMemory(segemntAddress, DkmReadMemoryFlags.None, sgegment);

                //bucket_count
                var bucket_count = (int)BitConverter.ToUInt64(sgegment, 144);//Evaluate(visualizedExpression, $"{rootExpr.FullName}.{path}.bucket_count_.value()");

                path =path + ".buckets_.value()->";

                //Buckets*
                var buckets = BitConverter.ToUInt64(sgegment, 128);

                var Buckets = new byte[32+8* bucket_count];
                TargetProcess.ReadMemory(buckets, DkmReadMemoryFlags.None, Buckets);

                //hazptr_obj_base_linked 40 + void*  + void*
                var NodeTSize = 64;

                for(int bucket_i = 0; bucket_i != bucket_count;++bucket_i)
                {
                    var nodepath = path + $"buckets_[{bucket_i}].link_.value()";
                    var next = BitConverter.ToUInt64(Buckets, 32 + 8 * bucket_i);

                    while(next != 0)
                    {
                        var itempath = nodepath + "->item_.item_";
                        var NodeT = new byte[NodeTSize];
                        TargetProcess.ReadMemory(next, DkmReadMemoryFlags.None, NodeT);

                        var children = BitConverter.ToUInt64(NodeT, 48);
                        childrens.Add(itempath);

                        next = BitConverter.ToUInt64(NodeT, 40);
                        nodepath =nodepath + "->next_.value()";
                    }
                }
            }

            item.ChildrenPath = childrens.ToArray();
        }

        void IDkmCustomVisualizer.GetItems(DkmVisualizedExpression visualizedExpression, DkmEvaluationResultEnumContext enumContext, int startIndex, int count, out DkmChildVisualizedExpression[] items)
        {
            var item = visualizedExpression.GetDataItem<HashMapDataItem>();
            var rootExpr = (DkmRootVisualizedExpression)visualizedExpression;

            items = new DkmChildVisualizedExpression[count];

            for (int i = 0; i!= count;++i)
            {
                var path = item.ChildrenPath[startIndex + i];

                var evaluationResult = Evaluate(visualizedExpression, $"{rootExpr.FullName}.{path}");

                //childRoot.EvaluateVisualizedExpression(out var evaluationResult);
                var indexResult = DkmSuccessEvaluationResult.Create(
                     visualizedExpression.InspectionContext,
                     visualizedExpression.StackFrame,
                     $"[{i}]",
                     $"{rootExpr.FullName}[{i}]",
                     DkmEvaluationResultFlags.ReadOnly,
                     evaluationResult.Value,
                     null,
                     evaluationResult.Type,
                     evaluationResult.Category,
                     evaluationResult.Access,
                     evaluationResult.StorageType,
                     evaluationResult.TypeModifierFlags,
                     evaluationResult.Address,
                     evaluationResult.CustomUIVisualizers,
                     evaluationResult.ExternalModules,
                     null
                    );

                items[i] = DkmChildVisualizedExpression.Create(
                    visualizedExpression.InspectionContext,
                    visualizedExpression.VisualizerId, visualizedExpression.SourceId, visualizedExpression.StackFrame,
                    rootExpr.ValueHome, indexResult, visualizedExpression,
                    (uint)(startIndex+i),null
                    );
            }
        }

        string IDkmCustomVisualizer.GetUnderlyingString(DkmVisualizedExpression visualizedExpression)
        {
            throw new System.NotImplementedException();
        }

        void IDkmCustomVisualizer.SetValueAsString(DkmVisualizedExpression visualizedExpression, string value, int timeout, out string errorText)
        {
            throw new System.NotImplementedException();
        }

        void IDkmCustomVisualizer.UseDefaultEvaluationBehavior(DkmVisualizedExpression visualizedExpression, out bool useDefaultEvaluationBehavior, out DkmEvaluationResult defaultEvaluationResult)
        {
            defaultEvaluationResult = null;
            useDefaultEvaluationBehavior = false;
        }
    }
}
